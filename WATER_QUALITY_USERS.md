# Water Quality Monitoring - User Access

## Admin User Credentials

### Water Quality Monitoring Admin
- **Name**: Water Quality Monitoring Admin
- **Email**: `wqmadmin@mail.com`
- **Password**: `wqmadmin2025***`
- **Role**: Admin
- **Purpose**: View and manage water quality monitoring data

## Other System Users

### Main Admin User
- **Name**: Admin User
- **Email**: `homeadmin@mail.com`
- **Password**: `Passworrd2025***`
- **Role**: Admin
- **Purpose**: General system administration

### Test User (ESP32)
- **Name**: Test User
- **Email**: `esp32test@example.com`
- **Password**: `esp32password2025`
- **Role**: Regular User
- **Purpose**: Testing ESP32 door lock integration

## User Management

### Creating the WQ Admin User
The Water Quality Monitoring Admin user is automatically created when you run:
```bash
php artisan db:seed --class=WaterQualityAdminSeeder
```

### Recreating All Users
To recreate all users (if database is fresh):
```bash
php artisan db:seed
```

### Manual User Creation
If you need to create the user manually:
```bash
php artisan tinker
>>> App\Models\User::create([
...     'name' => 'Water Quality Monitoring Admin',
...     'email' => 'wqmadmin@mail.com',
...     'password' => bcrypt('wqmadmin2025***'),
...     'is_admin' => true,
...     'email_verified_at' => now()
... ]);
```

## Login URLs

### Web Interface
- **URL**: `http://your-domain.com/login`
- **Smart Dashboard**: `http://your-domain.com/dashboard` (redirects based on user role)
- **Water Quality Dashboard**: `http://your-domain.com/water-quality/dashboard` ✨ **NEW**
- **Devices Management**: `http://your-domain.com/water-quality/devices` ✨ **NEW**

### API Authentication
For API access using Sanctum tokens:
```bash
curl -X POST http://your-domain.com/api/login \
  -H "Content-Type: application/json" \
  -d '{
    "email": "wqmadmin@mail.com",
    "password": "wqmadmin2025***"
  }'
```

## User Permissions & Role-Based Access ✨ **NEW**

### Water Quality Admin (`wqmadmin@mail.com`)
- ✅ **Dedicated Water Quality Dashboard** with real-time sensor data
- ✅ **Device Management** - view all monitoring devices
- ✅ **Historical Data Access** - charts, trends, and data export
- ✅ **Alert Monitoring** - view all water quality alerts
- ✅ **CSV Export** - download sensor data for analysis
- 🔄 **Auto-redirected** to water quality dashboard on login

### Door Lock Admin (`homeadmin@mail.com`)
- ✅ **Door Lock Control Panel** - ESP32 command interface
- ✅ **RFID Management** - if implemented
- ✅ **Door Status Monitoring**
- 🚪 **Focused** on door lock system only

### Super Admins (`is_admin = true`)
- ✅ **Full System Access** - both water quality and door locks
- ✅ **Cross-system Navigation** - can switch between dashboards
- ✅ **All Admin Functions** available

### Regular Users (`is_admin = false`)
- ✅ **Basic Dashboard Access**
- ❌ **No Water Quality Admin Functions**
- ❌ **No Door Lock Admin Functions**

## Security Notes

1. **Change default passwords** in production
2. **Enable 2FA** if available (Fortify is configured)
3. **Use HTTPS** in production
4. **Regular password rotation** recommended
5. **Monitor login attempts** for security

## Database Structure

Users are stored in the `users` table with these key fields:
- `id` - Primary key
- `name` - Full name
- `email` - Email address (unique)
- `password` - Hashed password
- `is_admin` - Boolean flag for admin privileges
- `email_verified_at` - Timestamp for email verification
- `created_at` / `updated_at` - Timestamps

## ✅ Implemented Features

1. ✅ **Water Quality Dashboard** - Complete with real-time data, charts, and statistics
2. ✅ **Role-Based Access Control** - Separate interfaces for different user types  
3. ✅ **Device Management** - View and monitor all water quality devices
4. ✅ **Data Visualization** - Interactive charts with Chart.js integration
5. ✅ **Data Export** - CSV export functionality for data analysis
6. ✅ **Smart Navigation** - Role-based sidebar navigation
7. ✅ **Auto-Redirect** - Users automatically go to their appropriate dashboard

## Next Steps (Optional Enhancements)

1. **Real-Time Updates** - WebSocket integration for live data updates
2. **Email Notifications** - Alert system for critical water quality issues  
3. **User Management Interface** - Web-based user administration
4. **Mobile Responsive** - Optimize for mobile device viewing
5. **Data Analytics** - Advanced reporting and trend analysis