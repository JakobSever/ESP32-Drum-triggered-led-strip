# ESP32 Drum Kit with LED Visualizer and Bluetooth Control

This project showcases an interactive drum kit system built with an ESP32 microcontroller. It features Bluetooth connectivity via RemoteXY, an LED strip for visual effects, and a piezo sensor to detect drum hits.

## Features

- **ESP32 Microcontroller**: The heart of the system, handling all the operations and communication.
- **Bluetooth Connectivity**: Utilizes the RemoteXY library for easy Bluetooth-based control via a mobile app.
- **LED Strip Visualizer**: Displays colorful visual effects synchronized with drum hits.
- **Piezo Sensor**: Detects the impact when the drum is hit and triggers corresponding actions.
- **Customizable**: Easily modify the code and configuration to suit your preferences and hardware setup.

## Components

- **ESP32 Development Board**
- **Piezo Sensor**: For detecting drum hits.
- **LED Strip**: For visual feedback and effects.
- **RemoteXY**: Mobile app interface for Bluetooth control.

## Getting Started

### Prerequisites

- **ESP32 Development Board**
- **Piezo Sensor**
- **LED Strip**
- **RemoteXY App** (available on both Android and iOS)

### Installation

1. **Clone the Repository**:
    ```bash
    git clone https://github.com/JakobSever/ESP32-Drum-triggered-led-strip.git
    cd ESP32-Drum-triggered-led-strip
    ```

2. **Install the Required Libraries**:
    - **ESP32 Board Support**: Follow the instructions [here](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html) to set up the ESP32 board in the Arduino IDE.
    - **RemoteXY Library**: Install the RemoteXY library from the Arduino Library Manager.

3. **Configure the RemoteXY Interface**:
    - Open the RemoteXY app and design your interface.
    - Generate the RemoteXY code and replace the placeholder in the project code with your generated code.

4. **Upload the Code**:
    - Connect your ESP32 board to your computer.
    - Open the project in the Arduino IDE.
    - Select the correct board and port.
    - Upload the code to the ESP32.

### Usage

- **Connect via Bluetooth**:
    - Open the RemoteXY app on your mobile device.
    - Connect to the ESP32 via Bluetooth.
    - Use the app to control and configure the LED effects.

- **Play the Drum**:
    - Hit the drum equipped with the piezo sensor.
    - Observe the LED strip displaying visual effects in response to the hits.

## Customization

Feel free to customize the code and hardware setup to better suit your needs. You can add more sensors, change the LED patterns, or enhance the RemoteXY interface for additional controls.

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request for any enhancements or bug fixes.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Thanks to the creators of the ESP32 and RemoteXY for their excellent platforms and tools.
- Inspired by various open-source drum kit projects and DIY LED visualizer ideas.

---

Feel free to tweak the content to better fit your specific project details or personal preferences!
