# Smart Door Lock System

A comprehensive IoT solution for remote door lock control using Laravel API backend and ESP32 microcontroller. This system enables secure, real-time communication between a web dashboard and physical door locks via WiFi.

## Features

- **Web Dashboard**: User-friendly interface for sending lock/unlock/status commands
- **RESTful API**: Secure endpoints for command management and status updates
- **ESP32 Integration**: Microcontroller polls for commands and controls relays/LEDs
- **API Authentication**: Key-based authentication for ESP32 communication
- **Real-time Status**: Live feedback on command execution and door status
- **Mobile Responsive**: Optimized for desktop and mobile devices
- **WiFi Reconnection**: Automatic reconnection handling for ESP32

## Tech Stack

- **Backend**: Laravel 11 (PHP)
- **Database**: SQLite/MySQL
- **Frontend**: Blade templates with Tailwind CSS
- **Microcontroller**: ESP32 with Arduino IDE
- **Communication**: HTTP REST API with JSON
- **Authentication**: API key middleware

## Installation

### Prerequisites
- PHP 8.1+
- Composer
- Node.js & NPM
- Arduino IDE with ESP32 board support
- ESP32 microcontroller

### Backend Setup
1. Clone the repository:
   ```bash
   git clone https://github.com/ginxFromYt/ARDUINO_API.git
   cd ARDUINO_API
   ```

2. Install PHP dependencies:
   ```bash
   composer install
   ```

3. Install Node dependencies:
   ```bash
   npm install
   ```

4. Copy environment file:
   ```bash
   cp .env.example .env
   ```

5. Generate application key:
   ```bash
   php artisan key:generate
   ```

6. Set up database (SQLite by default):
   ```bash
   php artisan migrate --seed
   ```

7. Build assets:
   ```bash
   npm run build
   ```

8. Start the server:
   ```bash
   php artisan serve --host=0.0.0.0 --port=8000
   ```

### ESP32 Setup
1. Install Arduino IDE and ESP32 board support
2. Open the provided ESP32 sketch
3. Update WiFi credentials and API details
4. Upload to ESP32 board

## API Endpoints

### Authentication Required (Web)
- `POST /api/login` - User authentication
- `POST /api/door/command` - Send command (lock/unlock/status)
- `GET /api/door/status` - Get door status

### ESP32 Endpoints (API Key Required)
- `GET /api/door/command` - Fetch pending commands
- `POST /api/door/status` - Update door status

### Public
- `GET /` - Welcome page

## Hardware Wiring

### ESP32 Pinout
- GPIO 23: Relay control (door lock mechanism)
- GPIO 19: Green LED (locked indicator)
- GPIO 18: Red LED (unlocked indicator)
- GPIO 5: White LED (status indicator)
- GPIO 2: Built-in LED (fallback)

### Relay Connection
- S (Signal): ESP32 GPIO 23
- + (VCC): ESP32 5V
- - (GND): ESP32 GND
- COM/NO: Connect to solenoid lock

### LED Connections
- Anode (+): GPIO via 330Ω resistor
- Cathode (-): ESP32 GND

## Usage

1. Access the web dashboard at `http://localhost:8000/dashboard`
2. Log in with seeded user credentials
3. Use the ESP32 Control Panel to send commands
4. ESP32 will poll for commands every 5 seconds
5. Commands are executed with LED/relay feedback
6. Status updates are sent back to the API

## Configuration

### Environment Variables
- `APP_NAME`: Application name
- `APP_ENV`: Environment (local/production)
- `APP_KEY`: Laravel app key
- `DB_CONNECTION`: Database type
- `ESP32_API_KEY`: API key for ESP32 authentication

### ESP32 Configuration
- Update `ssid` and `password` for WiFi
- Set `serverIP` to your Laravel server IP
- Ensure `apiKey` matches `.env` value

## Security

- API key authentication for ESP32 endpoints
- CSRF protection on web forms
- Input validation on all endpoints
- Secure password hashing

## Development

### Running Tests
```bash
php artisan test
```

### Code Style
```bash
./vendor/bin/pint
```

### Building Assets
```bash
npm run dev
npm run build
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Support

For issues and questions:
- Open an issue on GitHub
- Check the documentation
- Review the code comments

---

**Note**: This is an educational/demo project. For production use, implement additional security measures and hardware safeguards.
