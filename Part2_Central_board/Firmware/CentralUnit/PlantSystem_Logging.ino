

static char log_print_buffer[512];

int vprintf_into_spiffs(const char* szFormat, va_list args) {
	//write evaluated format string into buffer
	int ret = vsnprintf (log_print_buffer, sizeof(log_print_buffer), szFormat, args);

	//output is now in buffer. write to file.
	if(ret >= 0) {
    if(!SPIFFS.exists("/log.txt")) 
    {
      File writeLog = SPIFFS.open("/log.txt", FILE_WRITE);
      if(!writeLog) Serial.println("Couldn't open log.txt"); 
      delay(50);
      writeLog.close();
    }
    
		File spiffsLogFile = SPIFFS.open("/log.txt", FILE_APPEND);
		//debug output
		//printf("[Writing to SPIFFS] %.*s", ret, log_print_buffer);
		spiffsLogFile.write((uint8_t*) log_print_buffer, (size_t) ret);
		//to be safe in case of crashes: flush the output
		spiffsLogFile.flush();
		spiffsLogFile.close();
	}
	return ret;
}



void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}


void LOG_Timestamp(void)
{
	time_t now;
	char strftime_buf[64];
	struct tm timeinfo;

	time(&now);
	// Set timezone to China Standard Time
	setenv("TZ", "CST-8", 1);
	tzset();

	localtime_r(&now, &timeinfo);
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
	esp_log_write(ESP_LOG_INFO, "SKWS", "-- Date/time is: %s", strftime_buf);
}

void ps_debug_log_reset_Reason(RESET_REASON reason)
{
  switch ( reason)
  {
    case 1  : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: POWERON_RESET\n");break;          /**<1,  Vbat power on reset*/
    case 3  : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: SW_RESET\n");break;               /**<3,  Software reset digital core*/
    case 4  : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: OWDT_RESET\n");break;             /**<4,  Legacy watch dog reset digital core*/
    case 5  : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: DEEPSLEEP_RESET\n");break;        /**<5,  Deep Sleep reset digital core*/
    case 6  : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: SDIO_RESET\n");break;             /**<6,  Reset by SLC module, reset digital core*/
    case 7  : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: TG0WDT_SYS_RESET\n");break;       /**<7,  Timer Group0 Watch dog reset digital core*/
    case 8  : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: TG1WDT_SYS_RESET\n");break;       /**<8,  Timer Group1 Watch dog reset digital core*/
    case 9  : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: RTCWDT_SYS_RESET\n");break;       /**<9,  RTC Watch dog Reset digital core*/
    case 10 : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: INTRUSION_RESET\n");break;       /**<10, Instrusion tested to reset CPU*/
    case 11 : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: TGWDT_CPU_RESET\n");break;       /**<11, Time Group reset CPU*/
    case 12 : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: SW_CPU_RESET\n");break;          /**<12, Software reset CPU*/
    case 13 : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: RTCWDT_CPU_RESET\n");break;      /**<13, RTC Watch dog Reset CPU*/
    case 14 : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: EXT_CPU_RESET\n");break;         /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: RTCWDT_BROWN_OUT_RESET\n");break;/**<15, Reset when the vdd voltage is not stable*/
    case 16 : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: RTCWDT_RTC_RESET\n");break;      /**<16, RTC Watch dog reset digital core and rtc module*/
    default : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: NO_MEAN\n");
  }
}

void ps_debug_log_verbose_reset_reason(RESET_REASON reason)
{
  switch ( reason)
  {
    case 1  : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: Vbat power on reset\n");break;
    case 3  : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: Software reset digital core\n");break;
    case 4  : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: Legacy watch dog reset digital core\n");break;
    case 5  : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: Deep Sleep reset digital core\n");break;
    case 6  : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: Reset by SLC module, reset digital core\n");break;
    case 7  : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: Timer Group0 Watch dog reset digital core\n");break;
    case 8  : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: Timer Group1 Watch dog reset digital core\n");break;
    case 9  : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: RTC Watch dog Reset digital core\n");break;
    case 10 : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: Instrusion tested to reset CPU\n");break;
    case 11 : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: Time Group reset CPU\n");break;
    case 12 : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: Software reset CPU\n");break;
    case 13 : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: RTC Watch dog Reset CPU\n");break;
    case 14 : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: for APP CPU, reseted by PRO CPU\n");break;
    case 15 : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: Reset when the vdd voltage is not stable\n");break;
    case 16 : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: RTC Watch dog reset digital core and rtc module\n");break;
    default : esp_log_write(ESP_LOG_INFO, "SKWS", "Reset: NO_MEAN\n");
  }
}

