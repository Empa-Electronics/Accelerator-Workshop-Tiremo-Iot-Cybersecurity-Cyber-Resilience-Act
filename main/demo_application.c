#include "ap_manager.h"
#include "nvs_memory.h"
#include "network_controller.h"
#include "mqtt_handler.h"
#include "atca_config.h"
#include "hsm_manager.h"
#include "secure_mercek.h"
#include "sntp_controller.h"
#include "timer_handler.h"
#include "crypto_aes_ble.h"
#include "version.h"

static const char *TAG = "CYBERWHIZ MAIN";

void secureMercekCb (TimerHandle_t xTimer);

#define SECURE_MERCEK_PERIOD_IN_SECS 6

QueueHandle_t eventQueueHandle;

/// @brief Security Related Telemetry - IoT Core - HIDS
// void app_main(void)
// {
//     /* -------------------------------------------------------------
//      * 1. Print CyberWhiz library version
//      *    Helps identify SDK / library compatibility during runtime.
//      * ------------------------------------------------------------- */
//     ESP_LOGI(TAG,
//              "CyberWhiz Library Version: %d.%d.%d",
//              NB_VERSION_MAJOR,
//              NB_VERSION_MINOR,
//              NB_VERSION_BUILD);

//     /* -------------------------------------------------------------
//      * 2. Initialize NVS (Non-Volatile Storage)
//      *    Used for storing configuration and persistent data.
//      * ------------------------------------------------------------- */
//     ESP_ERROR_CHECK(nvs_init());

//     /* -------------------------------------------------------------
//      * 3. Initialize Wi-Fi / TCP-IP stack
//      *    Prepares network interfaces and event loop.
//      * ------------------------------------------------------------- */
//     ESP_ERROR_CHECK(wifi_init());

//     /* -------------------------------------------------------------
//      * 4. Initialize Secure Mercek module
//      *
//      *    - Initializes security related telemetry buffers
//      * ------------------------------------------------------------- */
//     ESP_ERROR_CHECK(init_secure_mercek());

//     /* -------------------------------------------------------------
//      * 5. Connect to Wi-Fi Access Point
//      *
//      *    - Device joins local network
//      *    - Required before any cloud or time synchronization
//      * ------------------------------------------------------------- */
//     wifi_init_sta("EMPA_ARGE_4G", "Empa1982");

//     /* -------------------------------------------------------------
//      * 6. Start SNTP time synchronization
//      *
//      *    - Required for:
//      *        * Accurate telemetry timestamps
//      * ------------------------------------------------------------- */
//     sntp_start_with_timer(TIME_SYNC_PERIOD_IN_SECS);

//     /* -------------------------------------------------------------
//      * 7. Establish secure MQTT connection to IoT Core
//      *
//      *    - Initializes Hardware Security Module (HSM)
//      *    - Uses TLS
//      *    - Client authentication performed inside HSM
//      *    - Private key never leaves secure hardware
//      *    - Connects to AWS IoT Core
//      * ------------------------------------------------------------- */
//     mqtt_connect();

//     /* -------------------------------------------------------------
//      * 8. Start secure port tracking task
//      *
//      *    - Monitors network activity
//      *    - Detects anomalous behavior
//      * ------------------------------------------------------------- */
//     start_port_tracking_task();

//     /* -------------------------------------------------------------
//      * 9. Start Secure Mercek periodic telemetry timer
//      *
//      *    - Collects security metrics
//      *    - Sends secure telemetry to IoT Core at fixed intervals
//      * ------------------------------------------------------------- */
//     start_named_timer(
//         "secureMercekTimer",
//         SECURE_MERCEK_PERIOD_IN_SECS,
//         secureMercekCb,
//         true
//     );

//     /* -------------------------------------------------------------
//      * 10. Keep main task alive
//      *
//      *    - All functionality runs in background tasks / timers
//      *    - app_main remains idle
//      * ------------------------------------------------------------- */
//     vTaskDelay(portMAX_DELAY);
// }

// /// @brief Security related telemetry send callback
// /// @param xTimer 
// void secureMercekCb(TimerHandle_t xTimer){
//     (void)xTimer;

//     send_secure_mercek();
// }

/// @brief Encrypted Wi-Fi Credential Storage Demo 
void app_main(void)
{
    ESP_LOGI(TAG, "CyberWhiz Library Version: %d.%d.%d", NB_VERSION_MAJOR, NB_VERSION_MINOR, NB_VERSION_BUILD);

    /* -------------------------------------------------------------
     * 1. Initialize NVS (Non-Volatile Storage)
     *    This is required to store encrypted credentials persistently.
     * ------------------------------------------------------------- */
    ESP_ERROR_CHECK(nvs_init());

    /* -------------------------------------------------------------
     * 2. Initialize Wi-Fi and TCP/IP stack
     *    Prepares ESP32 networking subsystem.
     * ------------------------------------------------------------- */
    ESP_ERROR_CHECK(wifi_init());

    /* -------------------------------------------------------------
     * 3. Plaintext Wi-Fi credentials (for demo purposes)
     *    In real products, these should never be hardcoded.
     * ------------------------------------------------------------- */
    const char* SSID = "EMPA_ARGE_4G";
    const char* Password = "Empa1982";

    /* -------------------------------------------------------------
     * 4. Log plaintext credentials in HEX
     *    (Only for workshop visibility – not recommended in production)
     * ------------------------------------------------------------- */
    ESP_LOGI(TAG, "Plain SSID:");
    ESP_LOG_BUFFER_HEX(TAG, SSID, strlen(SSID) + 1);

    ESP_LOGI(TAG, "Plain Password:");
    ESP_LOG_BUFFER_HEX(TAG, Password, strlen(Password) + 1);

    /* -------------------------------------------------------------
     * 5. Buffers for encryption / decryption
     * ------------------------------------------------------------- */
    uint8_t encryptedData[256] = {0};    // Holds encrypted output
    uint8_t decryptedData[256] = {0};    // Holds decrypted plaintext
    uint8_t messageTag[16] = {0};        // AES-GCM authentication tag
    uint8_t messageIV[12] = {0};         // AES-GCM IV (96-bit recommended)
    uint8_t encryptionKey[32];           // 256-bit AES key

    /* -------------------------------------------------------------
     * 6. Read diversified encryption key from Secure Element (ATECC)
     * ------------------------------------------------------------- */
    if (ATCA_SUCCESS != getDivirsifiedKey(encryptionKey))
    {
        ESP_LOGE(TAG, "Diversified Key Read Error!");
        return;
    }

    /* -------------------------------------------------------------
     * 7. Generate random IV using hardware RNG (HSM)
     * ------------------------------------------------------------- */
    if (ATCA_SUCCESS != getRandomNumberHSM(messageIV, sizeof(messageIV)))
    {
        ESP_LOGE(TAG, "Random Number Read Error!");
        return;
    }

    /* -------------------------------------------------------------
     * 8. Encrypt SSID using AES-GCM
     * ------------------------------------------------------------- */
    aes_gcm_encrypt(
        (const uint8_t*)SSID,            // Plaintext
        strlen(SSID),                    // Length (no null terminator)
        encryptionKey,                   // AES-256 key
        messageIV,                       // IV
        sizeof(messageIV),               // IV length
        encryptedData,                   // Ciphertext output
        messageTag                       // Authentication tag
    );

    ESP_LOGI(TAG, "Encrypted SSID:");
    ESP_LOG_BUFFER_HEX(TAG, encryptedData, strlen(SSID));

    /* -------------------------------------------------------------
     * 9. Store encrypted SSID, IV and TAG in NVS
     * ------------------------------------------------------------- */
    write_blob_to_nvs("enc_ssid_key", encryptedData, strlen(SSID));
    write_blob_to_nvs("enc_ssid_iv", messageIV, sizeof(messageIV));
    write_blob_to_nvs("enc_ssid_tag", messageTag, sizeof(messageTag));

    /* -------------------------------------------------------------
     * 10. Clear sensitive buffers from RAM
     * ------------------------------------------------------------- */
    memset(encryptedData, 0, sizeof(encryptedData));
    memset(messageTag, 0, sizeof(messageTag));
    memset(messageIV, 0, sizeof(messageIV));

    /* -------------------------------------------------------------
     * 11. Generate a NEW IV for password encryption
     *    (Never reuse IV with the same key!)
     * ------------------------------------------------------------- */
    if (ATCA_SUCCESS != getRandomNumberHSM(messageIV, sizeof(messageIV)))
    {
        ESP_LOGE(TAG, "Random Number Read Error!");
        return;
    }

    /* -------------------------------------------------------------
     * 12. Encrypt Wi-Fi Password using AES-GCM
     * ------------------------------------------------------------- */
    aes_gcm_encrypt(
        (const uint8_t*)Password,
        strlen(Password),
        encryptionKey,
        messageIV,
        sizeof(messageIV),
        encryptedData,
        messageTag
    );

    ESP_LOGI(TAG, "Encrypted Password:");
    ESP_LOG_BUFFER_HEX(TAG, encryptedData, strlen(Password));

    /* -------------------------------------------------------------
     * 13. Store encrypted password, IV and TAG in NVS
     * ------------------------------------------------------------- */
    write_blob_to_nvs("enc_pw_key", encryptedData, strlen(Password));
    write_blob_to_nvs("enc_pw_iv", messageIV, sizeof(messageIV));
    write_blob_to_nvs("enc_pw_tag", messageTag, sizeof(messageTag));

    /* =============================================================
     * 14. READ BACK encrypted credentials from NVS
     *     Decrypt them and connect to Wi-Fi
     * ============================================================= */

    char readSSID[32];
    char readPassword[64];

    size_t inputBufferSize = sizeof(encryptedData);
    size_t ivBufferSize = sizeof(messageIV);
    size_t tagBufferSize = sizeof(messageTag);

    /* -------------------------------------------------------------
     * 15. Read encrypted SSID data from NVS
     * ------------------------------------------------------------- */
    read_blob_from_nvs("enc_ssid_key", encryptedData, &inputBufferSize);
    read_blob_from_nvs("enc_ssid_iv", messageIV, &ivBufferSize);
    read_blob_from_nvs("enc_ssid_tag", messageTag, &tagBufferSize);

    /* -------------------------------------------------------------
     * 16. Decrypt SSID
     *     AES-GCM automatically verifies integrity using TAG
     * ------------------------------------------------------------- */
    aes_gcm_decrypt(
        (const uint8_t *)encryptedData,
        inputBufferSize,
        encryptionKey,
        messageIV,
        sizeof(messageIV),
        messageTag,
        decryptedData
    );

    /* -------------------------------------------------------------
     * 17. Convert decrypted SSID to C-string
     * ------------------------------------------------------------- */
    strncpy(readSSID, (char*)decryptedData, sizeof(readSSID) - 1);
    readSSID[sizeof(readSSID) - 1] = '\0';

    ESP_LOGI(TAG, "Decrypted SSID:");
    ESP_LOG_BUFFER_HEX(TAG, decryptedData, strlen(readSSID));

    /* -------------------------------------------------------------
     * 18. Clear buffers again before password handling
     * ------------------------------------------------------------- */
    memset(encryptedData, 0, sizeof(encryptedData));
    memset(decryptedData, 0, sizeof(decryptedData));
    memset(messageTag, 0, sizeof(messageTag));
    memset(messageIV, 0, sizeof(messageIV));

    inputBufferSize = sizeof(encryptedData);
    ivBufferSize = sizeof(messageIV);
    tagBufferSize = sizeof(messageTag);

    /* -------------------------------------------------------------
     * 19. Read encrypted password data from NVS
     * ------------------------------------------------------------- */
    read_blob_from_nvs("enc_pw_key", encryptedData, &inputBufferSize);
    read_blob_from_nvs("enc_pw_iv", messageIV, &ivBufferSize);
    read_blob_from_nvs("enc_pw_tag", messageTag, &tagBufferSize);

    /* -------------------------------------------------------------
     * 20. Decrypt password
     * ------------------------------------------------------------- */
    aes_gcm_decrypt(
        (const uint8_t *)encryptedData,
        inputBufferSize,
        encryptionKey,
        messageIV,
        sizeof(messageIV),
        messageTag,
        decryptedData
    );

    strncpy(readPassword, (char*)decryptedData, sizeof(readPassword) - 1);
    readPassword[sizeof(readPassword) - 1] = '\0';

    ESP_LOGI(TAG, "Decrypted Password:");
    ESP_LOG_BUFFER_HEX(TAG, decryptedData, strlen(readPassword));

    /* -------------------------------------------------------------
     * 21. Connect to Wi-Fi using decrypted credentials
     * ------------------------------------------------------------- */
    wifi_init_sta(readSSID, readPassword);
}
