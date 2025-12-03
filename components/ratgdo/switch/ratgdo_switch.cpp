#include "ratgdo_switch.h"
#include "../ratgdo_state.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ratgdo {

    static const char* const TAG = "ratgdo.switch";

    void RATGDOSwitch::dump_config()
    {
        LOG_SWITCH("", "RATGDO Switch", this);
        if (this->switch_type_ == SwitchType::RATGDO_LEARN) {
            ESP_LOGCONFIG(TAG, "  Type: Learn");
        } else if (this->switch_type_ == SwitchType::RATGDO_CLOSE_NOTIFICATION) {
            ESP_LOGCONFIG(TAG, "  Type: Close Notification");
        } else if (this->switch_type_ == SwitchType::RATGDO_invert_obstruction) {
            ESP_LOGCONFIG(TAG, "  Type: Obstruction Invert");
        }
    }

    void RATGDOSwitch::setup()
    {
        if (this->switch_type_ == SwitchType::RATGDO_LEARN) {
            this->parent_->subscribe_learn_state([this](LearnState state) {
                this->publish_state(state == LearnState::ACTIVE);
            });
        } else if (this->switch_type_ == SwitchType::RATGDO_LED) {
            this->pin_->setup();
            this->parent_->subscribe_vehicle_arriving_state([this](VehicleArrivingState state) {
                this->write_state(state == VehicleArrivingState::YES);
            });
        } else if (this->switch_type_ == SwitchType::RATGDO_CLOSE_NOTIFICATION) {
            // Setup persistence
            this->pref_ = global_preferences->make_preference<bool>(this->get_object_id_hash());

            bool state = false;
            if (this->pref_.load(&state)) {
                // Restore saved state
                this->parent_->set_close_notification_enabled(state);
                this->publish_state(state);
            } else {
                // Default to disabled
                this->publish_state(false);
            }
        } else if (this->switch_type_ == SwitchType::RATGDO_invert_obstruction) {
            // Setup persistence
            this->pref_ = global_preferences->make_preference<bool>(this->get_object_id_hash());

            bool state = false;
            if (this->pref_.load(&state)) {
                // Restore saved state
                this->parent_->set_invert_obstructioned(state);
                this->publish_state(state);
            } else {
                // Default to disabled
                this->publish_state(false);
            }
        }
    }

    void RATGDOSwitch::write_state(bool state)
    {
        if (this->switch_type_ == SwitchType::RATGDO_LEARN) {
            if (state) {
                this->parent_->activate_learn();
            } else {
                this->parent_->inactivate_learn();
            }
        } else if (this->switch_type_ == SwitchType::RATGDO_LED) {
            this->pin_->digital_write(state);
            this->publish_state(state);
        } else if (this->switch_type_ == SwitchType::RATGDO_CLOSE_NOTIFICATION) {
            this->parent_->set_close_notification_enabled(state);
            // Save to flash
            this->pref_.save(&state);
            this->publish_state(state);
        } else if (this->switch_type_ == SwitchType::RATGDO_invert_obstruction) {
            this->parent_->set_invert_obstructioned(state);
            // Save to flash
            this->pref_.save(&state);
            this->publish_state(state);
        }
    }

} // namespace ratgdo
} // namespace esphome
