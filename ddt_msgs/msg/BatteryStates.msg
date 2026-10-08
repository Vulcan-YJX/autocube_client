# Extended battery state message.
# The common fields follow sensor_msgs/msg/BatteryState semantics.

std_msgs/Header header

float32 voltage
float32 temperature
float32 current
float32 charge
float32 capacity
float32 design_capacity
float32 percentage

uint8 POWER_SUPPLY_STATUS_UNKNOWN=0
uint8 POWER_SUPPLY_STATUS_CHARGING=1
uint8 POWER_SUPPLY_STATUS_DISCHARGING=2
uint8 POWER_SUPPLY_STATUS_NOT_CHARGING=3
uint8 POWER_SUPPLY_STATUS_FULL=4
uint8 power_supply_status

uint8 POWER_SUPPLY_HEALTH_UNKNOWN=0
uint8 POWER_SUPPLY_HEALTH_GOOD=1
uint8 POWER_SUPPLY_HEALTH_OVERHEAT=2
uint8 POWER_SUPPLY_HEALTH_DEAD=3
uint8 POWER_SUPPLY_HEALTH_OVERVOLTAGE=4
uint8 POWER_SUPPLY_HEALTH_UNSPEC_FAILURE=5
uint8 POWER_SUPPLY_HEALTH_COLD=6
uint8 POWER_SUPPLY_HEALTH_WATCHDOG_TIMER_EXPIRE=7
uint8 POWER_SUPPLY_HEALTH_SAFETY_TIMER_EXPIRE=8
uint8 power_supply_health

uint8 POWER_SUPPLY_TECHNOLOGY_UNKNOWN=0
uint8 POWER_SUPPLY_TECHNOLOGY_NIMH=1
uint8 POWER_SUPPLY_TECHNOLOGY_LION=2
uint8 POWER_SUPPLY_TECHNOLOGY_LIPO=3
uint8 POWER_SUPPLY_TECHNOLOGY_LIFE=4
uint8 POWER_SUPPLY_TECHNOLOGY_NICD=5
uint8 POWER_SUPPLY_TECHNOLOGY_LIMN=6
uint8 power_supply_technology

bool present
float32[] cell_voltage
float32[] cell_temperature
string location
string serial_number

# Battery management system fields.
uint32 app_version
uint32 build_timestamp
uint32 pack_status
uint32 bat_status
uint8 battery_id
uint16 cell_min
uint16 cell_max
uint16 cell_diff_max
uint64 data_timestamp
uint8[3] uuid
uint8 pack_idx
uint8 esoc
bool self_lock_key_status
bool pms_key2_status
string version
uint32 cycle_count
