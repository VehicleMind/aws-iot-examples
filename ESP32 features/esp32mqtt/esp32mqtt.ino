#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

void connectAWSIoT();
void mqttCallback (char* topic, byte* payload, unsigned int length);

char *ssid = "VehicleMindMtl";
char *password = "DisruptAuto2018!";

const char *endpoint = "a5dh679nok7v3.iot.us-east-1.amazonaws.com";
const int port = 8883;
char *pubTopic = "$aws/things/FTX006/shadow/update";
char *subTopic = "$aws/things/FTX006/shadow/update/delta";
char *pubTopic2 = "vm/FTX006/engine/temp";

const char* rootCA = \
"-----BEGIN CERTIFICATE-----\n"
"MIIE0zCCA7ugAwIBAgIQGNrRniZ96LtKIVjNzGs7SjANBgkqhkiG9w0BAQUFADCB\n"
"yjELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQL\n"
"ExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJp\n"
"U2lnbiwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxW\n"
"ZXJpU2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0\n"
"aG9yaXR5IC0gRzUwHhcNMDYxMTA4MDAwMDAwWhcNMzYwNzE2MjM1OTU5WjCByjEL\n"
"MAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQLExZW\n"
"ZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJpU2ln\n"
"biwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxWZXJp\n"
"U2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0aG9y\n"
"aXR5IC0gRzUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCvJAgIKXo1\n"
"nmAMqudLO07cfLw8RRy7K+D+KQL5VwijZIUVJ/XxrcgxiV0i6CqqpkKzj/i5Vbex\n"
"t0uz/o9+B1fs70PbZmIVYc9gDaTY3vjgw2IIPVQT60nKWVSFJuUrjxuf6/WhkcIz\n"
"SdhDY2pSS9KP6HBRTdGJaXvHcPaz3BJ023tdS1bTlr8Vd6Gw9KIl8q8ckmcY5fQG\n"
"BO+QueQA5N06tRn/Arr0PO7gi+s3i+z016zy9vA9r911kTMZHRxAy3QkGSGT2RT+\n"
"rCpSx4/VBEnkjWNHiDxpg8v+R70rfk/Fla4OndTRQ8Bnc+MUCH7lP59zuDMKz10/\n"
"NIeWiu5T6CUVAgMBAAGjgbIwga8wDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8E\n"
"BAMCAQYwbQYIKwYBBQUHAQwEYTBfoV2gWzBZMFcwVRYJaW1hZ2UvZ2lmMCEwHzAH\n"
"BgUrDgMCGgQUj+XTGoasjY5rw8+AatRIGCx7GS4wJRYjaHR0cDovL2xvZ28udmVy\n"
"aXNpZ24uY29tL3ZzbG9nby5naWYwHQYDVR0OBBYEFH/TZafC3ey78DAJ80M5+gKv\n"
"MzEzMA0GCSqGSIb3DQEBBQUAA4IBAQCTJEowX2LP2BqYLz3q3JktvXf2pXkiOOzE\n"
"p6B4Eq1iDkVwZMXnl2YtmAl+X6/WzChl8gGqCBpH3vn5fJJaCGkgDdk+bW48DW7Y\n"
"5gaRQBi5+MHt39tBquCWIMnNZBU4gcmU7qKEKQsTb47bDN0lAtukixlE0kF6BWlK\n"
"WE9gyn6CagsCqiUXObXbf+eEZSqVir2G3l6BFoMtEMze/aiCKm0oHw0LxOXnGiYZ\n"
"4fQRbxC1lfznQgUy286dUV4otp6F01vvpX1FQHKOtw5rDgb7MzVIcbidJ4vEZV8N\n"
"hnacRHr2lVz2XTIIM6RUthg/aFzyQkqFOFSDX9HoLPKsEdao7WNq\n"
"-----END CERTIFICATE-----\n";

const char* certificate = \
"-----BEGIN CERTIFICATE-----\n"
"MIIDWTCCAkGgAwIBAgIUaYpOB4lwNZkq8A3fA24xAiIjpQowDQYJKoZIhvcNAQEL\n"
"BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n"
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTE3MTAxNjIyNDEx\n"
"MFoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n"
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAIisU3PSVqmUzLqISb61\n"
"XmVCxUcO08wGgYQfjfYEuJhDcC05ttbZp1RgpGQfulhxJo8PQVvAQ0c1vGSQsSNb\n"
"TsBuHjGUA1D6+OZBnm2GPJnIOg8sKihVpmj0oe3ZsSy1e+76y7v/AW7pDWN8VbVw\n"
"1y2mghMOyQAEY5ktW0baFRvNKqB6rZ3DPIkGFPHDpd05IltUfqufrhatObzOhzmr\n"
"84skHKtKuEwLpwcy/JwVGB44cGct1EYIrCvIb1lbTF84GEqpIXl4h4KVh0qLs/0r\n"
"YFDppj6LT3Rfgxv/rfcgS9p93tYMCohCX4pCKdd1c59IK9kL26ztVXaKh9NksY8p\n"
"mm8CAwEAAaNgMF4wHwYDVR0jBBgwFoAU5v4p+HyZRvc2f1Sj+OEgZxCM3HEwHQYD\n"
"VR0OBBYEFD+YktGJT8vHorZ47ayTbNZQIVFwMAwGA1UdEwEB/wQCMAAwDgYDVR0P\n"
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBIxzHGQOpg+24hx2Kdp0GCOHrm\n"
"YiO6CkE6hZbAw87FWz4tEocmGgV1WgUFOXEIGX9BjKtj6a8QOEswhtCb2n1ghpP8\n"
"95seiVINGWQmLgEomYf3nR0CBjS3WQmvQJzQx2gzAFEsH2jZ9sejVya1KVcFEsm5\n"
"rWCCkGbhY9bbCqvVYel+9NByopfiNMVk2LM45dF9XF4jpAdAueQeOdcoFnY7aWFS\n"
"qL/iPCid02rFM0/KUL0GTzERkprHAr7jLCtTeEa5Ind7NKo7Y67dpX/7zRqAaWCt\n"
"hPSgAwqEd+StskrAGFVT47RJdFOd5N2h0pnSRqrsgK74vKlokQQ6LmfA7sMT\n"
"-----END CERTIFICATE-----\n";

const char* privateKey = \
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEowIBAAKCAQEAiKxTc9JWqZTMuohJvrVeZULFRw7TzAaBhB+N9gS4mENwLTm2\n"
"1tmnVGCkZB+6WHEmjw9BW8BDRzW8ZJCxI1tOwG4eMZQDUPr45kGebYY8mcg6Dywq\n"
"KFWmaPSh7dmxLLV77vrLu/8BbukNY3xVtXDXLaaCEw7JAARjmS1bRtoVG80qoHqt\n"
"ncM8iQYU8cOl3TkiW1R+q5+uFq05vM6HOavziyQcq0q4TAunBzL8nBUYHjhwZy3U\n"
"RgisK8hvWVtMXzgYSqkheXiHgpWHSouz/StgUOmmPotPdF+DG/+t9yBL2n3e1gwK\n"
"iEJfikIp13Vzn0gr2QvbrO1VdoqH02SxjymabwIDAQABAoIBAFMM5GktF5xU31M6\n"
"HhXVAFdoC5jyWaFREsLGqe0lUUa1NCVHPOOxvwx27W4qGYJEv7mO+5hVbIyJCsHR\n"
"atKhWWl1gMN/vcQnvbetiCfluk14bk9p9vQrl8OPZBa0ggU5AuZlYAgNHUHYPFqH\n"
"400uFJOY6tHJ2jo4cI1UEnMrckhBq0P9Zm9+piOVyNXwJkKrg002N2HMKrxjjAW3\n"
"Dhan7hqIeiYUXGv3CgamQzdXV3VLBhsbmv/S/10ud3yW4Q2GC7Z3eEPasxiMAtyM\n"
"wPkHR6MmMoC8V+S/Nl1XwDoMSbgsYUrSVeBuIfHYEAeGZ4eBwU+I63O+PTVkNdLA\n"
"FmdvbwECgYEA6FX4oKCmn3hlSg/CT9nW2AZvrJQVPSoDFUje3g+aoNZE0dd6X0EB\n"
"k+buQ1+lnSLlPCr8uNvsSjPiR2DMwbBCkYoWf0U0iARD5/arD5iTQ0cWRqkkABNN\n"
"+5QoPLGXAMgqWwu3Gnk8riXRPXPsPyLPCDrHwiYNvoVHB8T9uf5ATlcCgYEAlpgA\n"
"zQjN5SZ1g7yCVmgQsJ0jU1N7I2vnfcWvJdYZ7p17fsb3hMDcRE3u4ZHWi98GTMcI\n"
"IexwHCFc0WaemhVAck6XedAryHJKc8QXlNM14o0UVwCVVSHEuuBOj7RUthzHKb+C\n"
"So0X/ETnZcHpyiZtxMemWysRDjph+a+jE7MFVakCgYEAwEh+h3nNzMdYlO3r9Cgm\n"
"rgzNmbfIyCEgAhqYEJI7eAc9V6oM5g6n3p9N68aaCy2ZKK2vENM6gMLl8AsGCvr0\n"
"bP0q11QQQM2+4Zh9RGAS6KhJoFtVpxZTAPZCQtD7VzNSEp3kgW17MemsfT6WdD47\n"
"t+Tl9CGIWHh3K8/16VuqHusCgYB4ZdZeFmfmiLTzOcdKpbCSQ0920wtj6mUNXPsG\n"
"2QGqkCWn2hMSA88WnOgbV6mb8hbrU0ThOlL3aoT2tYCf19XKjaL1NMWMYDO+Ekx4\n"
"I9S05+4XENRTV8gdUC//HEGNx7xDWGUmjV9bxQrrg/kSMV8ZMDWENg5QvcSPByju\n"
"GvZs4QKBgBasR4Rfm0QGUKAtDp+nCDFDK0oQAtwgdejbCjQbsYxbzl50YfQB53I1\n"
"ksFjwaJwIkkC2au9bItR+2VE4mv7qiS1ACucBl9N3NFRqxzhDvtY7iQSBJZ0783g\n"
"GHZHNkzaS54yJezfDJn+AT6PHVd4+6V2nwRWEbApq6NBN/iUSrTu\n"
"-----END RSA PRIVATE KEY-----\n";

WiFiClientSecure httpsClient;
PubSubClient mqttClient(httpsClient);

void setup() {
    delay(1000);
    Serial.begin(115200);

    // Start WiFi
    Serial.println("Connecting to ");
    Serial.print(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected.");

    // Configure MQTT Client
    httpsClient.setCACert(rootCA);
    httpsClient.setCertificate(certificate);
    httpsClient.setPrivateKey(privateKey);
    mqttClient.setServer(endpoint, port);
    mqttClient.setCallback(mqttCallback);

    connectAWSIoT();
}

void connectAWSIoT() {
    while (!mqttClient.connected()) {
        if (mqttClient.connect("ESP32_device")) {
            Serial.println("Connected.");
            int qos = 0;
            mqttClient.subscribe(subTopic, qos);
            Serial.println("Subscribed.");
        } else {
            Serial.print("Failed. Error state=");
            Serial.print(mqttClient.state());
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

long messageSentAt = 0;
int dummyValue = 0;
char pubMessage[128];

void mqttCallback (char* topic, byte* payload, unsigned int length) {
    Serial.print("Received. topic=");
    Serial.println(topic);
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.print("\n");
}

void mqttLoop() {
    if (!mqttClient.connected()) {
        connectAWSIoT();
    }
    mqttClient.loop();

    long now = millis();
    if (now - messageSentAt > 5000) {
        messageSentAt = now;
        sprintf(pubMessage, "{\"state\": {\"desired\":{\"foo\":\"%d\"}}}", dummyValue++);
        Serial.print("Publishing message to topic ");
        Serial.println(pubTopic2);
        Serial.println(pubMessage);
        mqttClient.publish(pubTopic2, pubMessage);
        Serial.println("Published.");
    }
}

void loop() {
  mqttLoop();
}
