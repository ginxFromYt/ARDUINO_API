# Fix Git Merge Conflict - Server Commands

## Current Situation
You have an unresolved merge conflict in `public/.htaccess` that's preventing git operations.

## Step-by-Step Solution

### Step 1: Check the conflict status
```bash
cd arduino-api.ginxproduction.com
git status
```

### Step 2: View the conflicted file
```bash
cat public/.htaccess
```
Look for conflict markers like:
```
<<<<<<< HEAD
(your local changes)
=======
(incoming changes)
>>>>>>> branch-name
```

### Step 3: Resolve the conflict manually
Edit the `.htaccess` file to resolve conflicts:
```bash
nano public/.htaccess
# or
vi public/.htaccess
```

**Remove the conflict markers** and keep the content you want.

### Step 4: Mark the conflict as resolved
```bash
git add public/.htaccess
```

### Step 5: Complete the merge
```bash
git commit -m "Resolve .htaccess merge conflict"
```

### Step 6: Now you can pull the latest changes
```bash
git pull origin main
```

---

## Alternative: Reset and Clean Pull (CAUTION: This will lose local .htaccess changes)

If you don't need your local `.htaccess` changes:

### Option A: Hard reset (loses ALL local changes)
```bash
git reset --hard HEAD
git clean -fd
git pull origin main
```

### Option B: Just reset the conflicted file
```bash
git checkout HEAD -- public/.htaccess
git pull origin main
```

---

## Safe Approach: Backup and Reset

### Step 1: Backup your current .htaccess
```bash
cp public/.htaccess public/.htaccess.backup
```

### Step 2: Reset the conflicted file
```bash
git checkout HEAD -- public/.htaccess
```

### Step 3: Pull the updates
```bash
git pull origin main
```

### Step 4: Compare and merge manually if needed
```bash
diff public/.htaccess.backup public/.htaccess
```

---

## Complete Clean Pull Sequence (Recommended)

Run these commands in order:

```bash
# 1. Navigate to your project
cd arduino-api.ginxproduction.com

# 2. Check current status
git status

# 3. Backup your .htaccess if you need it
cp public/.htaccess public/.htaccess.backup

# 4. Reset the conflicted file
git checkout HEAD -- public/.htaccess

# 5. Pull the latest changes
git pull origin main

# 6. If needed, restore your custom .htaccess settings
# Compare: diff public/.htaccess.backup public/.htaccess
# Then manually edit public/.htaccess to add back your custom rules

# 7. Run Laravel commands
php artisan migrate
php artisan db:seed --class=WaterQualityAdminSeeder
php artisan config:clear
php artisan route:clear
php artisan cache:clear
```

---

## After Successful Pull - Setup Commands

```bash
# Set your API key in .env (replace with your actual key)
echo "API_KEY=your-secure-api-key-here" >> .env

# Run migrations for water quality system
php artisan migrate

# Create the water quality admin user
php artisan db:seed --class=WaterQualityAdminSeeder

# Clear all caches
php artisan config:clear
php artisan route:clear
php artisan cache:clear
php artisan view:clear
```

---

## What You'll Get After Pull

✅ **Complete Water Quality Monitoring System**
- Dedicated dashboard for `wqmadmin@mail.com`
- Device management interface
- Real-time data visualization
- CSV export functionality
- Role-based navigation

✅ **Enhanced API System** 
- Improved middleware security
- Better error handling
- Multiple authentication methods

✅ **Updated Arduino Code**
- GPRS HTTP POST integration
- Enhanced sensor data transmission

---

## Troubleshooting

### If you still get conflicts:
```bash
# Force reset everything (CAUTION: loses all local changes)
git reset --hard origin/main
git clean -fd
```

### If pull still fails:
```bash
# Check what's blocking
git status
git diff

# Remove any remaining problematic files
git checkout -- .
git pull origin main
```

Choose the approach that best fits your needs - the "Safe Approach" is recommended if you have custom .htaccess rules you want to preserve.