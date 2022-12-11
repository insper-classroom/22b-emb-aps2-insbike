/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include <asf.h>
#include <string.h>
#include "ili9341.h"
#include "img1.h"
#include "img2.h"
#include "img3.h"
#include "img4.h"
#include "logo_pequeno.h"
#include "background.h"
#include "lvgl.h"
#include "touch/touch.h"

LV_FONT_DECLARE(dseg10);
LV_FONT_DECLARE(roboto20);
LV_FONT_DECLARE(roboto15);
LV_IMG_DECLARE(background);
LV_IMG_DECLARE(logo_pequeno);

#include "arm_math.h"

#define TASK_SIMULATOR_STACK_SIZE (4096 / sizeof(portSTACK_TYPE))
#define TASK_SIMULATOR_STACK_PRIORITY (tskIDLE_PRIORITY)

#define RAIO 0.508/2
#define VEL_MAX_KMH  5.0f
#define VEL_MIN_KMH  0.5f

#define SENSOR_PIO		 PIOA
#define SENSOR_PIO_ID	 ID_PIOA
#define SENSOR_PIO_IDX	 19
#define SENSOR_PIO_IDX_MASK (1 << SENSOR_PIO_IDX)
//#define RAMP 

/************************************************************************/
/* LCD / LVGL                                                           */
/************************************************************************/

#define LV_HOR_RES_MAX          (240)
#define LV_VER_RES_MAX          (320)

/*A static or global variable to store the buffers*/
static lv_disp_draw_buf_t disp_buf;

/*Static or global buffer(s). The second buffer is optional*/
static lv_color_t buf_1[LV_HOR_RES_MAX * LV_VER_RES_MAX];
static lv_disp_drv_t disp_drv;          /*A variable to hold the drivers. Must be static or global.*/
static lv_indev_drv_t indev_drv;

static lv_obj_t * scr1;  // screen 1
QueueHandle_t xQueuedt;


typedef struct  {
  uint32_t year;
  uint32_t month;
  uint32_t day;
  uint32_t week;
  uint32_t hour;
  uint32_t minute;
  uint32_t second;
} calendar;

SemaphoreHandle_t xSemaphoreHora;
/************************************************************************/
/* RTOS                                                                 */
/************************************************************************/

#define TASK_LCD_STACK_SIZE                (1024*6/sizeof(portSTACK_TYPE))
#define TASK_LCD_STACK_PRIORITY            (tskIDLE_PRIORITY)

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,  signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName) {
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	for (;;) {	}
}

extern void vApplicationIdleHook(void) { }

extern void vApplicationTickHook(void) { }

extern void vApplicationMallocFailedHook(void) {
	configASSERT( ( volatile void * ) NULL );
}
lv_obj_t * labelTempo;
lv_obj_t * labelDuration;
lv_obj_t * labelDistancia;
lv_obj_t * labelVMedia;
lv_obj_t * labelVInst;
/************************************************************************/
/* lvgl                                                                 */
/************************************************************************/

static void event_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked");
	}
	else if(code == LV_EVENT_VALUE_CHANGED) {
		LV_LOG_USER("Toggled");
	}
}
void RTC_init(Rtc *rtc, uint32_t id_rtc, calendar t, uint32_t irq_type) {
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(rtc, 0);

	/* Configura data e hora manualmente */
	rtc_set_date(rtc, t.year, t.month, t.day, t.week);
	rtc_set_time(rtc, t.hour, t.minute, t.second);

	/* Configure RTC interrupts */
	NVIC_DisableIRQ(id_rtc);
	NVIC_ClearPendingIRQ(id_rtc);
	NVIC_SetPriority(id_rtc, 4);
	NVIC_EnableIRQ(id_rtc);

	/* Ativa interrupcao via alarme */
	rtc_enable_interrupt(rtc,  irq_type);
}

void RTC_Handler(void) {
    uint32_t ul_status = rtc_get_status(RTC);
	
    /* seccond tick */
    if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {	
		// o código para irq de segundo vem aqui
		xSemaphoreGiveFromISR(xSemaphoreHora, 0);
    }

    /* Time or date alarm */
    if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
    	// o código para irq de alame vem aqui
    }

    rtc_clear_status(RTC, RTC_SCCR_SECCLR);
    rtc_clear_status(RTC, RTC_SCCR_ALRCLR);
    rtc_clear_status(RTC, RTC_SCCR_ACKCLR);
    rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
    rtc_clear_status(RTC, RTC_SCCR_CALCLR);
    rtc_clear_status(RTC, RTC_SCCR_TDERRCLR);
}

/**
* raio 20" => 50,8 cm (diametro) => 0.508/2 = 0.254m (raio)
* w = 2 pi f (m/s)
* v [km/h] = (w*r) / 3.6 = (2 pi f r) / 3.6
* f = v / (2 pi r 3.6)
* Exemplo : 5 km / h = 1.38 m/s
*           f = 0.87Hz
*           t = 1/f => 1/0.87 = 1,149s
*/
float kmh_to_hz(float vel, float raio) {
    float f = vel / (2*PI*raio*3.6);
    return(f);
}

static void task_simulador(void *pvParameters) {

    pmc_enable_periph_clk(ID_PIOC);
    pio_set_output(PIOC, PIO_PC31, 1, 0, 0);

    float vel = VEL_MAX_KMH;
    float f;
    int ramp_up = 1;

    while(1){
        pio_clear(PIOC, PIO_PC31);
        delay_ms(1);
        pio_set(PIOC, PIO_PC31);

        if (ramp_up) {
            printf("[SIMU] ACELERANDO: %d \n", (int) (10*vel));
            vel += 0.5;
        } else {
            printf("[SIMU] DESACELERANDO: %d \n",  (int) (10*vel));
            vel -= 0.5;
        }

        if (vel >= VEL_MAX_KMH)
        ramp_up = 0;
    else if (vel <= VEL_MIN_KMH)
    ramp_up = 1;
//#ifndef RAMP
        // vel = 5;
        // printf("[SIMU] CONSTANTE: %d \n", (int) (10*vel));
//#endif
        f = kmh_to_hz(vel, RAIO);
        int t = 965*(1.0/f); //UTILIZADO 965 como multiplicador ao invés de 1000
                             //para compensar o atraso gerado pelo Escalonador do freeRTOS
        delay_ms(t);
    }
}

static void pause_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked");
	}
}

static void play_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked");
	}
}

static void refresh_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked");
	}
}

static void list_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked");
	}
}

static void settings_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked");
	}
}
static void RTT_init(float freqPrescale, uint32_t IrqNPulses,
uint32_t rttIRQSource) {

	uint16_t pllPreScale = (int)(((float)32768) / freqPrescale);

	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);

	if (rttIRQSource & RTT_MR_ALMIEN) {
		uint32_t ul_previous_time;
		ul_previous_time = rtt_read_timer_value(RTT);
		while (ul_previous_time == rtt_read_timer_value(RTT))
		;
		rtt_write_alarm_time(RTT, IrqNPulses + ul_previous_time);
	}

	/* config NVIC */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 4);
	NVIC_EnableIRQ(RTT_IRQn);

	/* Enable RTT interrupt */
	if (rttIRQSource & (RTT_MR_RTTINCIEN | RTT_MR_ALMIEN))
	rtt_enable_interrupt(RTT, rttIRQSource);
	else
	rtt_disable_interrupt(RTT, RTT_MR_RTTINCIEN | RTT_MR_ALMIEN);
}

void RTT_Handler(void) {
  uint32_t ul_status;
  ul_status = rtt_get_status(RTT);

  /* IRQ due to Alarm */
  if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
	
   }  
}




void sensor_callback(void){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	int dt = rtt_read_timer_value(RTT);
	printf("rtt %d \n", dt);
	xQueueSendFromISR(xQueuedt,&dt,&xHigherPriorityTaskWoken);
	RTT_init(1000,1000,NULL);
}

static void io_init(void)
{
	/* Configure speed sensor input */
	pio_configure(SENSOR_PIO, PIO_INPUT, SENSOR_PIO_IDX_MASK, PIO_DEFAULT);
	pio_set_debounce_filter(SENSOR_PIO, SENSOR_PIO_IDX_MASK, 60);
	pio_handler_set(SENSOR_PIO, SENSOR_PIO_ID, SENSOR_PIO_IDX_MASK, PIO_IT_FALL_EDGE, &sensor_callback);
	pio_enable_interrupt(SENSOR_PIO, SENSOR_PIO_IDX_MASK);
	pio_get_interrupt_status(SENSOR_PIO);
	NVIC_EnableIRQ(SENSOR_PIO_ID);
	NVIC_SetPriority(SENSOR_PIO_ID, 4);
}
/************************************************************************/
/* TASKS                                                                */
/************************************************************************/
void lv_screen(void){

	lv_obj_t * background1 = lv_img_create(scr1);
	lv_img_set_src(background1, &background);
	lv_obj_align(background1, LV_ALIGN_CENTER, 0, 0);

	static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_bg_color(&style, lv_palette_darken(LV_PALETTE_GREY, 4));
    lv_style_set_border_color(&style, lv_palette_darken(LV_PALETTE_GREY, 4));
    lv_style_set_border_width(&style, 5);

	labelTempo = lv_label_create(scr1);
	lv_obj_align(labelTempo, LV_ALIGN_TOP_RIGHT, -5 , 5);
	lv_obj_set_style_text_font(labelTempo, &roboto20, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(labelTempo, lv_color_white(), LV_STATE_DEFAULT);
	lv_label_set_text_fmt(labelTempo, "%02d:%02d:%02d", 17, 46, 24);

	lv_obj_t * logo = lv_img_create(scr1);
	lv_img_set_src(logo, &logo_pequeno);
	lv_obj_align(logo, LV_ALIGN_TOP_LEFT, 20, 5);

	lv_obj_t * labelPause;

    lv_obj_t * btnPause = lv_btn_create(scr1);
    lv_obj_add_event_cb(btnPause, pause_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btnPause, LV_ALIGN_TOP_LEFT, 30, 80);
	lv_obj_add_style(btnPause, &style, 0);

    labelPause = lv_label_create(btnPause);
    lv_label_set_text(labelPause, LV_SYMBOL_PAUSE);
    lv_obj_center(labelPause);
	lv_obj_set_width(btnPause, 40);
	lv_obj_set_height(btnPause, 40);

	lv_obj_t * labelPlay;

    lv_obj_t * btnPlay = lv_btn_create(scr1);
    lv_obj_add_event_cb(btnPlay, play_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btnPlay, LV_ALIGN_TOP_RIGHT, -30, 80);
	lv_obj_add_style(btnPlay, &style, 0);

    labelPlay = lv_label_create(btnPlay);
    lv_label_set_text(labelPlay, LV_SYMBOL_PLAY);
    lv_obj_center(labelPlay);
	lv_obj_set_width(btnPlay, 40);
	lv_obj_set_height(btnPlay, 40);		

	lv_obj_t * labelRefresh;

    lv_obj_t * btnRefresh = lv_btn_create(scr1);
    lv_obj_add_event_cb(btnRefresh, refresh_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btnRefresh, LV_ALIGN_BOTTOM_LEFT, 30, -15);
	lv_obj_add_style(btnRefresh, &style, 0);

    labelRefresh = lv_label_create(btnRefresh);
    lv_label_set_text(labelRefresh, LV_SYMBOL_REFRESH);
    lv_obj_center(labelRefresh);
	lv_obj_set_width(btnRefresh, 40);
	lv_obj_set_height(btnRefresh, 40);	

	lv_obj_t * labelList;

    lv_obj_t * btnList = lv_btn_create(scr1);
    lv_obj_add_event_cb(btnList, list_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btnList, LV_ALIGN_BOTTOM_MID, 0, -15);
	lv_obj_add_style(btnList, &style, 0);

    labelList = lv_label_create(btnList);
    lv_label_set_text(labelList, LV_SYMBOL_LIST);
    lv_obj_center(labelList);
	lv_obj_set_width(btnList, 40);
	lv_obj_set_height(btnList, 40);		

	lv_obj_t * labelSettings;

    lv_obj_t * btnSettings = lv_btn_create(scr1);
    lv_obj_add_event_cb(btnSettings, settings_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btnSettings, LV_ALIGN_BOTTOM_RIGHT, -30, -15);
	lv_obj_add_style(btnSettings, &style, 0);

    labelSettings = lv_label_create(btnSettings);
    lv_label_set_text(labelSettings, LV_SYMBOL_SETTINGS);
    lv_obj_center(labelSettings);
	lv_obj_set_width(btnSettings, 40);
	lv_obj_set_height(btnSettings, 40);		

	labelDuration = lv_label_create(scr1);
	lv_obj_align(labelDuration, LV_ALIGN_CENTER, 0, 30);
	lv_obj_set_style_text_font(labelDuration, &roboto20, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(labelDuration, lv_color_white(), LV_STATE_DEFAULT);
	lv_label_set_text_fmt(labelDuration, "%02d:%02d:%02d", 0, 0, 0);

	labelDistancia = lv_label_create(scr1);
	lv_obj_align(labelDistancia, LV_ALIGN_CENTER, -50, 78);
	lv_obj_set_style_text_font(labelDistancia, &roboto20, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(labelDistancia, lv_color_white(), LV_STATE_DEFAULT);
	lv_label_set_text_fmt(labelDistancia, "%.1f", 0.0);

	labelVMedia = lv_label_create(scr1);
	lv_obj_align(labelVMedia, LV_ALIGN_CENTER, 50, 78);
	lv_obj_set_style_text_font(labelVMedia, &roboto20, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(labelVMedia, lv_color_white(), LV_STATE_DEFAULT);
	lv_label_set_text_fmt(labelVMedia, "%.1f", 0.0);

	labelVInst = lv_label_create(scr1);
	lv_obj_align(labelVInst, LV_ALIGN_CENTER, 0, -78);
	lv_obj_set_style_text_font(labelVInst, &roboto20, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(labelVInst, lv_color_white(), LV_STATE_DEFAULT);
	lv_label_set_text_fmt(labelVInst, "%.1f", 0.0);

	lv_obj_t * labelUp;

    lv_obj_t * btnUp = lv_btn_create(scr1);
    lv_obj_align(btnUp, LV_ALIGN_CENTER, 0, -38);
	lv_obj_add_style(btnUp, &style, 0);

    labelUp = lv_label_create(btnUp);
    lv_label_set_text(labelUp, LV_SYMBOL_UP);
    lv_obj_center(labelUp);
	lv_obj_set_width(btnUp, 10);
	lv_obj_set_height(btnUp, 10);

}

static void task_lcd(void *pvParameters) {
	int px, py;

	io_init();
	RTT_init(1000,1000,NULL);

	scr1  = lv_obj_create(NULL);
	lv_screen();
	lv_scr_load(scr1);
	int dt = 0;
	for (;;)  {
		if (xQueueReceive(xQueuedt, &dt, 0)) {
			//limitei a 100 a temperatura maxima
			// 100 -- 4092
			//  x -- value
			printf("rtt dt %d", dt);
		}
		lv_tick_inc(50);
		lv_task_handler();
		vTaskDelay(50);
	}
}

static void task_rtc(void *pvParameters) {
	calendar rtc_initial = {2018, 3, 19, 12, 15, 45 ,1};

	/** Configura RTC */
	RTC_init(RTC, ID_RTC, rtc_initial, RTC_IER_SECEN);
	
	/* Leitura do valor atual do RTC */
	uint32_t current_hour, current_min, current_sec;
	uint32_t current_year, current_month, current_day, current_week;
	rtc_get_time(RTC, &current_hour, &current_min, &current_sec);
	rtc_get_date(RTC, &current_year, &current_month, &current_day, &current_week);

	for (;;)  {
		if (xSemaphoreTake(xSemaphoreHora, 0) == pdTRUE){
			rtc_get_time(RTC, &current_hour, &current_min, &current_sec);
			rtc_get_date(RTC, &current_year, &current_month, &current_day, &current_week);
			lv_label_set_text_fmt(labelTempo, "%02d:%02d:%02d", current_hour, current_min, current_sec);
		}
	}
}
/************************************************************************/
/* configs                                                              */
/************************************************************************/

static void configure_lcd(void) {
	/**LCD pin configure on SPI*/
	pio_configure_pin(LCD_SPI_MISO_PIO, LCD_SPI_MISO_FLAGS);  //
	pio_configure_pin(LCD_SPI_MOSI_PIO, LCD_SPI_MOSI_FLAGS);
	pio_configure_pin(LCD_SPI_SPCK_PIO, LCD_SPI_SPCK_FLAGS);
	pio_configure_pin(LCD_SPI_NPCS_PIO, LCD_SPI_NPCS_FLAGS);
	pio_configure_pin(LCD_SPI_RESET_PIO, LCD_SPI_RESET_FLAGS);
	pio_configure_pin(LCD_SPI_CDS_PIO, LCD_SPI_CDS_FLAGS);
	
	ili9341_init();
	ili9341_backlight_on();
}

static void configure_console(void) {
	const usart_serial_options_t uart_serial_options = {
		.baudrate = USART_SERIAL_EXAMPLE_BAUDRATE,
		.charlength = USART_SERIAL_CHAR_LENGTH,
		.paritytype = USART_SERIAL_PARITY,
		.stopbits = USART_SERIAL_STOP_BIT,
	};

	/* Configure console UART. */
	stdio_serial_init(CONSOLE_UART, &uart_serial_options);

	/* Specify that stdout should not be buffered. */
	setbuf(stdout, NULL);
}

/************************************************************************/
/* port lvgl                                                            */
/************************************************************************/

void my_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) {
	ili9341_set_top_left_limit(area->x1, area->y1);   ili9341_set_bottom_right_limit(area->x2, area->y2);
	ili9341_copy_pixels_to_screen(color_p,  (area->x2 + 1 - area->x1) * (area->y2 + 1 - area->y1));
	
	/* IMPORTANT!!!
	* Inform the graphics library that you are ready with the flushing*/
	lv_disp_flush_ready(disp_drv);
}

void my_input_read(lv_indev_drv_t * drv, lv_indev_data_t*data) {
	int px, py, pressed;
	
	if (readPoint(&px, &py))
		data->state = LV_INDEV_STATE_PRESSED;
	else
		data->state = LV_INDEV_STATE_RELEASED; 
	
	data->point.x = py;
	data->point.y = 320 - px;
}

void configure_lvgl(void) {
	lv_init();
	lv_disp_draw_buf_init(&disp_buf, buf_1, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX);
	
	lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
	disp_drv.draw_buf = &disp_buf;          /*Set an initialized buffer*/
	disp_drv.flush_cb = my_flush_cb;        /*Set a flush callback to draw to the display*/
	disp_drv.hor_res = LV_HOR_RES_MAX;      /*Set the horizontal resolution in pixels*/
	disp_drv.ver_res = LV_VER_RES_MAX;      /*Set the vertical resolution in pixels*/

	lv_disp_t * disp;
	disp = lv_disp_drv_register(&disp_drv); /*Register the driver and save the created display objects*/
	
	/* Init input on LVGL */
	lv_indev_drv_init(&indev_drv);
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	indev_drv.read_cb = my_input_read;
	lv_indev_t * my_indev = lv_indev_drv_register(&indev_drv);
}

/************************************************************************/
/* main                                                                 */
/************************************************************************/
int main(void) {
	/* board and sys init */
	board_init();
	sysclk_init();
	configure_console();

	/* LCd, touch and lvgl init*/
	configure_lcd();
	ili9341_set_orientation(ILI9341_FLIP_Y | ILI9341_SWITCH_XY);
	configure_touch();
	configure_lvgl();

	xQueuedt = xQueueCreate(32, sizeof(int));

	if (xQueuedt == NULL){
		printf("falha em criar a queue \n");
	}
	/* Create task to control oled */
	if (xTaskCreate(task_lcd, "LCD", TASK_LCD_STACK_SIZE, NULL, TASK_LCD_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create lcd task\r\n");
	}

	/* Create task to control oled */
	if (xTaskCreate(task_rtc, "rtc", TASK_LCD_STACK_SIZE, NULL, TASK_LCD_STACK_PRIORITY, NULL) != pdPASS) {
	  printf("Failed to create rtc task\r\n");
	}

	if (xTaskCreate(task_simulador, "SIMUL", TASK_SIMULATOR_STACK_SIZE, NULL, TASK_SIMULATOR_STACK_PRIORITY, NULL) != pdPASS) {
        printf("Failed to create lcd task\r\n");
    }

	/* Attempt to create a semaphore. */
	xSemaphoreHora = xSemaphoreCreateBinary();
	if (xSemaphoreHora == NULL)
		printf("falha em criar o semaforo \n");
	
	/* Start the scheduler. */
	vTaskStartScheduler();

	while(1){ }
}
