#include "ratgdo_sensor.h"
#include "../ratgdo_state.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ratgdo {

    static const char* const TAG = "ratgdo.sensor";
    static const int MIN_DISTANCE = 100;  // ignore bugs crawling on the distance sensor & dust protection film
    static const int MAX_DISTANCE = 4000; // default maximum distance

#ifdef USE_DISTANCE
    RATGDOSensor::RATGDOSensor() : distance_sensor_(&Wire, 0) {}
#endif

    void RATGDOSensor::setup()
    {
        if (this->ratgdo_sensor_type_ == RATGDOSensorType::RATGDO_OPENINGS) {
            this->parent_->subscribe_openings([=](uint16_t value) {
                this->publish_state(value);
            });
        } else if (this->ratgdo_sensor_type_ == RATGDOSensorType::RATGDO_PAIRED_DEVICES_TOTAL) {
            this->parent_->subscribe_paired_devices_total([=](uint16_t value) {
                this->publish_state(value);
            });
        } else if (this->ratgdo_sensor_type_ == RATGDOSensorType::RATGDO_PAIRED_REMOTES) {
            this->parent_->subscribe_paired_remotes([=](uint16_t value) {
                this->publish_state(value);
            });
        } else if (this->ratgdo_sensor_type_ == RATGDOSensorType::RATGDO_PAIRED_KEYPADS) {
            this->parent_->subscribe_paired_keypads([=](uint16_t value) {
                this->publish_state(value);
            });
        } else if (this->ratgdo_sensor_type_ == RATGDOSensorType::RATGDO_PAIRED_WALL_CONTROLS) {
            this->parent_->subscribe_paired_wall_controls([=](uint16_t value) {
                this->publish_state(value);
            });
        } else if (this->ratgdo_sensor_type_ == RATGDOSensorType::RATGDO_PAIRED_ACCESSORIES) {
            this->parent_->subscribe_paired_accessories([=](uint16_t value) {
                this->publish_state(value);
            });
        } else if (this->ratgdo_sensor_type_ == RATGDOSensorType::RATGDO_DISTANCE) {
#ifdef USE_DISTANCE
            VL53L1X distance_sensor_(&Wire, 0);
            VL53L1X_Error rc = VL53L1X_ERROR_NONE;
            Wire.begin(this->parent_->get_tof_sda_pin(), this->parent_->get_tof_scl_pin(), 400000);
            distance_sensor_.begin();
            distance_sensor_.VL53L1X_Off();
            delay(150); // Give the sensor time to boot up
            rc = distance_sensor_.InitSensor(0x53);
            ESP_LOG1(TAG, "ToF Sensor initialized with address: 0x%02X", distance_sensor_.getAddress());
            if (rc != VL53L1X_ERROR_NONE)
            {
                ESP_LOG1(TAG, "ToF Sensor failed to initialize error: %d", rc);
                Wire.end(); // Disable I2C pins
                return;
            }
            rc = distance_sensor_.VL53L1X_SetDistanceMode(3);
            if (rc != VL53L1X_ERROR_NONE)
            {
                ESP_LOG1(TAG, "VL53L1X_SetDistanceMode error: %d", rc);
                return;
            }
            rc = distance_sensor_.VL53L1X_StartRanging();
            if (rc != VL53L1X_ERROR_NONE)
            {
                ESP_LOG1(TAG, "VL53L1X_StartMeasurement error: %d", rc);
                return;
            }
            this->parent_->subscribe_distance_measurement([=](int16_t value) {
                this->publish_state(value);
            });
#endif
        }
    }

    void RATGDOSensor::dump_config()
    {
        LOG_SENSOR("", "RATGDO Sensor", this);
        if (this->ratgdo_sensor_type_ == RATGDOSensorType::RATGDO_OPENINGS) {
            ESP_LOGCONFIG(TAG, "  Type: Openings");
        } else if (this->ratgdo_sensor_type_ == RATGDOSensorType::RATGDO_PAIRED_DEVICES_TOTAL) {
            ESP_LOGCONFIG(TAG, "  Type: Paired Devices");
        } else if (this->ratgdo_sensor_type_ == RATGDOSensorType::RATGDO_PAIRED_REMOTES) {
            ESP_LOGCONFIG(TAG, "  Type: Paired Remotes");
        } else if (this->ratgdo_sensor_type_ == RATGDOSensorType::RATGDO_PAIRED_KEYPADS) {
            ESP_LOGCONFIG(TAG, "  Type: Paired Keypads");
        } else if (this->ratgdo_sensor_type_ == RATGDOSensorType::RATGDO_PAIRED_WALL_CONTROLS) {
            ESP_LOGCONFIG(TAG, "  Type: Paired Wall Controls");
        } else if (this->ratgdo_sensor_type_ == RATGDOSensorType::RATGDO_PAIRED_ACCESSORIES) {
            ESP_LOGCONFIG(TAG, "  Type: Paired Accessories");
        } else if (this->ratgdo_sensor_type_ == RATGDOSensorType::RATGDO_DISTANCE) {
            ESP_LOGCONFIG(TAG, "  Type: Distance");
        }
    }
#ifdef USE_DISTANCE
    void RATGDOSensor::loop()
    {
        uint32_t current_millis = millis();
        if (this->ratgdo_sensor_type_ == RATGDOSensorType::RATGDO_DISTANCE)
        {
            uint16_t distance;
            int status;
            status = distance_sensor_.VL53L1X_GetDistance(&distance);
            if (status != VL53L1X_ERROR_NONE)
            {
                ESP_LOG1(TAG, "VL53L1X_GetDistance error: %d", status);
                return;
            } else {
                if (distance > MIN_DISTANCE && distance < MAX_DISTANCE) {
                    this->parent_->set_distance_measurement(distance);
                }
                distance_sensor_.VL53L1X_ClearInterrupt();
            }
        }
    }
#endif
} // namespace ratgdo
} // namespace esphome