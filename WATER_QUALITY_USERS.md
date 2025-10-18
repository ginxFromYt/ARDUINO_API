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
- **Dashboard**: `http://your-domain.com/dashboard`
- **Water Quality Dashboard**: `http://your-domain.com/water-quality` (if implemented)

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

## User Permissions

### Admin Users (`is_admin = true`)
- ✅ View all water quality data
- ✅ Manage system settings
- ✅ Access admin dashboard
- ✅ View system logs
- ✅ Manage other users (if user management is implemented)

### Regular Users (`is_admin = false`)
- ✅ View own data only
- ✅ Basic dashboard access
- ❌ No admin functions

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

## Next Steps

1. **Implement water quality dashboard** for the admin user
2. **Set up role-based permissions** for different views
3. **Create API endpoints** for dashboard data
4. **Implement user management interface** (optional)
5. **Set up email notifications** for critical alerts