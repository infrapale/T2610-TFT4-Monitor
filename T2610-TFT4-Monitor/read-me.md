
            -----------               -----------               -----------
            | Pico 2W |               |Pimoroni |     SPI       | TFT 4"  |
(Ruuvi)---->|         |               |Pico     |<------------->| SD      |
            | RFM MCU |               |Plus 2   |               |         |
(WiFi)----->|         |               |         |               |         |
            |         | Serial2       | TFT MCU |               -----------
            |         | Sensor Data   |         |               ----------- 
(Serial1)-->|         |-------------->|         |  Serial2      | Clipper |
            |         | Serial2       |         |<------------->| 4G LTE  |
            |         | Control Data  |         |               -----------
            |         |<--------------|         |               -----------
            |         |               |         |     I2S       | 3W I2S  |
            |         |               |         |-------------->|         |
            |         |               |         |               |         |
            |         |               |         |               -----------
            |         |               |         |               -----------
            |         |               |         |     I2C       | RTC     |
            |         |               |         |<------------->|         |
            |         |               |         |               |         |
            |         |               -----------               -----------
            |         |               -----------  
            |         |      SPI      |         | 
            |         |<------------->|         |<- - - >[433MHz]
            -----------               -----------

Use Cases:
  1.  Receive RFM69 Sensor Data
    (sensor)-->[433Mhz]-->{RFM69}-->{RFM MCU}-->[Serial2]-->{TFT MCU}
        <S;Ulko;T;23.5;H;54;B;2.54>

  2.  Receive Ruuvi Tag  Sensor Data
    (Ruuvi)-->[BLE]-->{RFM MCU}-->[Serial2]-->{TFT MCU}
        <S;1FEA;T;24.5;H;44;B;2.99>
        Temporary name! To be named similarly to other sensors
      
  3.  Send RFM69 Control Data
    (sensor)-->[433Mhz]-->{RFM69}-->{RFM MCU}-->[Serial2]-->{TFT MCU}
    {TFT MCU}-->[Serial2]-->{RFM MCU}-->{RFM69}-->[433Mhz]-->(control unit)
        <R;RANTA;BTN1;PUMP;1>



