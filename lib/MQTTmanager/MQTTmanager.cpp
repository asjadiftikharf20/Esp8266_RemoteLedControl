
#include <MQTTmanager.h>
#include "Arduino_JSON.h"
#include "ArduinoJson.h"
#include <ArduinoHttpClient.h>
#include <ESP8266HTTPClient.h>

// Translate iot_configs.h defines into variables used by the sample
static const char* ssid = IOT_CONFIG_WIFI_SSID;
static const char* password = IOT_CONFIG_WIFI_PASSWORD;
static const char* host = IOT_CONFIG_IOTHUB_FQDN;
static const char* device_id = IOT_CONFIG_DEVICE_ID;
static const char* device_key = IOT_CONFIG_DEVICE_KEY;
static const int port = 8883;

// Memory allocated for the sample's variables and structures.
static WiFiClientSecure wifi_client;
static X509List cert((const char*)ca_pem);
static PubSubClient mqtt_client(wifi_client);
static az_iot_hub_client client;
static char sas_token[200];
static uint8_t signature[512];
static unsigned char encrypted_signature[32];
static char base64_decoded_device_key[32];
static unsigned long next_telemetry_send_time_ms = 0;
static char telemetry_topic[128];
static uint8_t telemetry_payload[512];
static uint32_t telemetry_send_count = 0;

String json_data;

unsigned long lastTelemetryTime = 0;
unsigned long lastTwinRequestTime = 0;
const unsigned long telemetryInterval = 10000;  // 10 seconds
const unsigned long twinRequestInterval = DEVICE_TWIN_REQUEST_INTERVAL_IN_HOURS * 60 * 60 * 1000;  // 10 hours in milliseconds


// Auxiliary functions

void connectToWiFi()
{
  Serial.begin(9600);
  Serial.println();
  Serial.print("Connecting to WIFI SSID ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.print("WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void initializeTime()
{
  Serial.print("Setting time using SNTP");

  configTime(-5 * 3600, 0, NTP_SERVERS);
  time_t now = time(NULL);
  while (now < 1510592825)
  {
    delay(500);
    Serial.print(".");
    now = time(NULL);
  }
  Serial.println("done!");
}

static char* getCurrentLocalTimeString()
{
  time_t now = time(NULL);
  return ctime(&now);
}

static void printCurrentTime()
{
  Serial.print("Current time: ");
  Serial.print(getCurrentLocalTimeString());
}

// bool stateReceived = false;

void receivedCallback(char* topic, byte* payload, unsigned int length)
{
    Serial.print("Received [");
    Serial.print(topic);
    Serial.print("]: ");
    for (unsigned int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println("");

    // Handle twin response
    if (strstr(topic, "$iothub/twin/res/200") != NULL)
    {
        // Parse the twin payload for the "desired" state
        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, payload, length);

        if (error)
        {
            Serial.println("Failed to parse twin document");
            return;
        }

        const char* state_value = doc["desired"]["state"]; // Adjust based on your twin structure

        if (state_value != NULL)
        {
            if (strcmp(state_value, "0") == 0)
            {
                digitalWrite(LED_PIN, LOW);
                Serial.println("LED turned off (from twin)");
                EEPROM.put(STATE_MESSAGE_EEPROM_ADDRESS, '0');  // Save state in EEPROM
            }
            else if (strcmp(state_value, "1") == 0)
            {
                digitalWrite(LED_PIN, HIGH);
                Serial.println("LED turned on (from twin)");
                EEPROM.put(STATE_MESSAGE_EEPROM_ADDRESS, '1');  // Save state in EEPROM
            }
            EEPROM.commit();
        }
    }

    // Handle twin patch update messages
    else if (strstr(topic, "$iothub/twin/PATCH/properties/desired") != NULL)
{
    // Parse the twin patch payload
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, payload, length);

    if (error)
    {
        Serial.println("Failed to parse twin patch");
        return;
    }

    // Debug: print the received payload
    Serial.print("Received twin patch payload: ");
    String payloadStr = String((char*)payload);
    Serial.println(payloadStr);

    // Access the 'state' property directly from the payload
    if (doc.containsKey("state"))
    {
        int state_value = doc["state"]; // 'state' is an integer based on the payload

        if (state_value == 0)
        {
            digitalWrite(LED_PIN, LOW);
            Serial.println("LED turned off (from twin patch)");
            EEPROM.put(STATE_MESSAGE_EEPROM_ADDRESS, '0');  // Save state in EEPROM
        }
        else if (state_value == 1)
        {
            digitalWrite(LED_PIN, HIGH);
            Serial.println("LED turned on (from twin patch)");
            EEPROM.put(STATE_MESSAGE_EEPROM_ADDRESS, '1');  // Save state in EEPROM
        }
        EEPROM.commit();  // Commit the EEPROM write
    }
    else
    {
        Serial.println("Failed to find 'state' in the twin patch");
    }
}

    // Handle cloud-to-device messages
else if (strstr(topic, "devices/Asjad-Led_testModule/messages/devicebound/") != NULL)
{
    // Debug: print the received payload for analysis
    Serial.print("Received C2D message: ");
    String payloadStr = String((char*)payload);
    Serial.println(payloadStr);

    // Parse the URL-encoded message (e.g., state=1)
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    // Find the position of "state="
    int stateIndex = message.indexOf("state=");

    if (stateIndex != -1)
    {
        // Extract the value of 'state' (character after 'state=')
        char state_value = message.charAt(stateIndex + 6); // 6 is the length of "state="

        Serial.print("Parsed state value: ");
        Serial.println(state_value);

        // Control the LED based on the parsed state value
        if (state_value == '0')
        {
            digitalWrite(LED_PIN, LOW);
            Serial.println("LED turned off (from C2D message)");
            EEPROM.put(STATE_MESSAGE_EEPROM_ADDRESS, '0');  // Save state in EEPROM
        }
        else if (state_value == '1')
        {
            digitalWrite(LED_PIN, HIGH);
            Serial.println("LED turned on (from C2D message)");
            EEPROM.put(STATE_MESSAGE_EEPROM_ADDRESS, '1');  // Save state in EEPROM
        }

        EEPROM.commit();  // Commit EEPROM changes
    }
    else
    {
        Serial.println("State parameter not found in C2D message");
    }
}
}



static void initializeClients()
{
    az_iot_hub_client_options options = az_iot_hub_client_options_default();
    options.user_agent = AZ_SPAN_FROM_STR(AZURE_SDK_CLIENT_USER_AGENT);

    wifi_client.setTrustAnchors(&cert);

    az_result init_result = az_iot_hub_client_init(
        &client,
        az_span_create((uint8_t*)host, strlen(host)),
        az_span_create((uint8_t*)device_id, strlen(device_id)),
        &options);

    if (az_result_failed(init_result))
    {
        Serial.print("Failed initializing Azure IoT Hub client, error code: ");
        Serial.println(init_result);
        return;
    }

    mqtt_client.setServer(host, port);
    mqtt_client.setCallback(receivedCallback);

    Serial.println("Azure IoT Hub client initialized successfully");
}


/*
 * @brief           Gets the number of seconds since UNIX epoch until now.
 * @return uint32_t Number of seconds.
 */
static uint32_t getSecondsSinceEpoch() { return (uint32_t)time(NULL); }

static int generateSasToken(char* sas_token, size_t size)
{
  az_span signature_span = az_span_create((uint8_t*)signature, sizeofarray(signature));
  az_span out_signature_span;
  // az_span encrypted_signature_span = az_span_create((uint8_t*)encrypted_signature, sizeofarray(encrypted_signature));

  uint32_t expiration = getSecondsSinceEpoch() + TWENTY_FOUR_HOURS_IN_SECS;

  // Get signature
  if (az_result_failed(az_iot_hub_client_sas_get_signature(
          &client, expiration, signature_span, &out_signature_span)))
  {
    Serial.println("Failed getting SAS signature");
    return 1;
  }

  // Base64-decode device key
  int base64_decoded_device_key_length
      = base64_decode_chars(device_key, strlen(device_key), base64_decoded_device_key);

  if (base64_decoded_device_key_length == 0)
  {
    Serial.println("Failed base64 decoding device key");
    return 1;
  }

  // SHA-256 encrypt
  br_hmac_key_context kc;
  br_hmac_key_init(
      &kc, &br_sha256_vtable, base64_decoded_device_key, base64_decoded_device_key_length);

  br_hmac_context hmac_ctx;
  br_hmac_init(&hmac_ctx, &kc, 32);
  br_hmac_update(&hmac_ctx, az_span_ptr(out_signature_span), az_span_size(out_signature_span));
  br_hmac_out(&hmac_ctx, encrypted_signature);

  // Base64 encode encrypted signature
  String b64enc_hmacsha256_signature = base64::encode(encrypted_signature, br_hmac_size(&hmac_ctx));

  az_span b64enc_hmacsha256_signature_span = az_span_create(
      (uint8_t*)b64enc_hmacsha256_signature.c_str(), b64enc_hmacsha256_signature.length());

  // URl-encode base64 encoded encrypted signature
  if (az_result_failed(az_iot_hub_client_sas_get_password(
          &client,
          expiration,
          b64enc_hmacsha256_signature_span,
          AZ_SPAN_EMPTY,
          sas_token,
          size,
          NULL)))
  {
    Serial.println("Failed getting SAS token");
    return 1;
  }

  return 0;
}

static int connectToAzureIoTHub()
{
  size_t client_id_length;
  char mqtt_client_id[128];
  if (az_result_failed(az_iot_hub_client_get_client_id(
          &client, mqtt_client_id, sizeof(mqtt_client_id) - 1, &client_id_length)))
  {
    Serial.println("Failed getting client id");
    return 1;
  }

  mqtt_client_id[client_id_length] = '\0';

  char mqtt_username[128];
  // Get the MQTT user name used to connect to IoT Hub
  if (az_result_failed(az_iot_hub_client_get_user_name(
          &client, mqtt_username, sizeofarray(mqtt_username), NULL)))
  {
    printf("Failed to get MQTT clientId, return code\n");
    return 1;
  }

  Serial.print("Client ID: ");
  Serial.println(mqtt_client_id);

  Serial.print("Username: ");
  Serial.println(mqtt_username);

  mqtt_client.setBufferSize(MQTT_PACKET_SIZE);

  while (!mqtt_client.connected())
  {
    // time_t now = time(NULL);

    Serial.print("MQTT connecting ... ");

    if (mqtt_client.connect(mqtt_client_id, mqtt_username, sas_token))
    {
      // Serial.println (sas_token);
      Serial.println("connected.");

      // requestDeviceTwin();
    }
    else
    {
      Serial.print("failed, status code =");
      Serial.print(mqtt_client.state());
      Serial.println(". Trying again in 5 seconds.");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  // mqtt_client.subscribe(AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_SUBSCRIBE_TOPIC);
  // Subscribe to device twin desired property changes
  mqtt_client.subscribe(AZ_IOT_HUB_CLIENT_TWIN_PATCH_SUBSCRIBE_TOPIC);
  mqtt_client.subscribe(AZ_IOT_HUB_CLIENT_C2D_SUBSCRIBE_TOPIC);

  return 0;
}

void establishConnect()
{
    EEPROM.begin(128);
    digitalWrite(LED_PIN, LOW);

  initializeTime();
  printCurrentTime();
  initializeClients();

  // The SAS token is valid for 24 hours as setted.
  // After 10 hours the device must be restarted, or the client won't be able
  // to connect/stay connected to the Azure IoT Hub.
  if (generateSasToken(sas_token, sizeofarray(sas_token)) != 0)
  {
    Serial.println("Failed generating MQTT password");
  }
  else
  {
    Serial.println(sas_token);
    connectToAzureIoTHub();

    lastTelemetryTime = millis();
    lastTwinRequestTime = millis();

    char state_value = '2';
      EEPROM.get(STATE_MESSAGE_EEPROM_ADDRESS, state_value);

      if (state_value == '0') {
      digitalWrite(LED_PIN, LOW);
     } 
      else if (state_value == '1') {
      digitalWrite(LED_PIN, HIGH);
     }
  }

  
}

static char* getTelemetryPayload()
{
  time_t now = time(NULL);
  // struct tm* timeinfo; // Removed unused variable
  localtime(&now);

  json_data = task_read_mpu6050_data();

  az_span temp_span = az_span_create(telemetry_payload, sizeof(telemetry_payload));
  temp_span = az_span_copy(temp_span, AZ_SPAN_FROM_STR("{ \"msgCount\": "));
  az_result result = az_span_u32toa(temp_span, telemetry_send_count++, &temp_span);
  if (az_result_failed(result))
  {
    Serial.println("Failed to convert telemetry count to string");
    return nullptr;
  }
  temp_span = az_span_copy(temp_span, AZ_SPAN_FROM_STR("\" }"));
  temp_span = az_span_copy(temp_span, az_span_create_from_str(const_cast<char*>(json_data.c_str())));
  temp_span = az_span_copy_u8(temp_span, '\0');

  return (char*)telemetry_payload;
}

static void sendTelemetry()
{
  
  // digitalWrite(LED_PIN, HIGH);
  Serial.print(millis());
  Serial.print(" ESP8266 Sending telemetry . . . \n");
  if (az_result_failed(az_iot_hub_client_telemetry_get_publish_topic(
          &client, NULL, telemetry_topic, sizeof(telemetry_topic), NULL)))
  {
    Serial.println("Failed az_iot_hub_client_telemetry_get_publish_topic");
    return;
  }

  mqtt_client.publish(telemetry_topic, getTelemetryPayload(), false);
  // Serial.println("OK");
  // digitalWrite(LED_PIN, LOW);
}



void MQTT_loop()
{
  unsigned long currentTime = millis();

  if (millis() > next_telemetry_send_time_ms)
  {
    // Check if connected, reconnect if needed.
    if (!mqtt_client.connected())
    {
      connectToWiFi();
      establishConnect();
      requestDeviceTwin();
    }  

  }

  if (currentTime - lastTelemetryTime >= telemetryInterval) {
    sendTelemetry();
    lastTelemetryTime = currentTime;
  }

  // Request Device Twin based on the interval set (e.g., every 10 hours)
  if (currentTime - lastTwinRequestTime >= twinRequestInterval) {
    requestDeviceTwin();
    lastTwinRequestTime = currentTime;
  }

  // MQTT loop must be called to process Device-to-Cloud and Cloud-to-Device.
  mqtt_client.loop();
  // delay(10000);
}






void requestDeviceTwin() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        String url = "https://iothubdevuae.azure-devices.net/twins/Asjad-Led_testModule?api-version=2020-09-30";
        String sasToken = "SharedAccessSignature sr=iothubdevuae.azure-devices.net&sig=wnUcCwIBxfNb2ZASrDmVDtDlFcHjgga3BOUpcBOZ2aw%3D&se=1758614845&skn=iothubowner";

        http.begin(wifi_client, url);
        http.addHeader("Authorization", sasToken);
        http.addHeader("Content-Type", "application/json");

        int httpResponseCode = http.GET();
        if (httpResponseCode > 0) {
            String payload = http.getString();
            Serial.println("Twin document received :)");
            // Serial.println(payload);

            // Parse JSON document
            DynamicJsonDocument doc(512);  // Adjust size as needed
            DeserializationError error = deserializeJson(doc, payload);

            if (error) {
                Serial.println("Failed to parse JSON document");
                return;
            }

            // Extract the state value
            int state_value = doc["properties"]["desired"]["state"];

            // Perform actions based on state value
            if (state_value == 0) {
                digitalWrite(LED_PIN, LOW);
                Serial.println("LED turned off (from twin)");
                EEPROM.put(STATE_MESSAGE_EEPROM_ADDRESS, '0');  // Save state in EEPROM
            } else if (state_value == 1) {
                digitalWrite(LED_PIN, HIGH);
                Serial.println("LED turned on (from twin)");
                EEPROM.put(STATE_MESSAGE_EEPROM_ADDRESS, '1');  // Save state in EEPROM
            }
            Serial.println(" ");
            EEPROM.commit();
        } else {
            Serial.printf("Error on HTTP request: %s\n", http.errorToString(httpResponseCode).c_str());
        }
        http.end();
    } else {
        Serial.println("WiFi not connected.");
    }
}