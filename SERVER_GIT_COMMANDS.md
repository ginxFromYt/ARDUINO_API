# Git Stash, Pull, and Pop Commands for Your Server

## Current Situation
You have local changes to `public/.htaccess` that conflict with the incoming changes from the repository.

## Solution: Stash → Pull → Pop

### Step 1: Stash your local changes
```bash
cd /home/ebjyuaia/arduino-api.ginxproduction.com  # or your correct path
git stash push -m "Save local .htaccess changes before pull"
```

### Step 2: Pull the latest changes
```bash
git pull origin main
```

### Step 3: Pop your stashed changes back
```bash
git stash pop
```

## Complete Command Sequence
Run these commands one by one on your server:

```bash
# Navigate to your project directory
cd arduino-api.ginxproduction.com

# Stash local changes
git stash push -m "Save local .htaccess changes before pull"

# Pull latest changes from GitHub
git pull origin main

# Restore your local changes
git stash pop
```

## Alternative: If you want to see what's stashed
```bash
# List all stashes
git stash list

# See what's in the latest stash
git stash show -p

# Apply stash without removing it (safer option)
git stash apply

# If everything looks good, then drop the stash
git stash drop
```

## If there are merge conflicts after stash pop:
1. Git will tell you which files have conflicts
2. Edit the conflicted files manually
3. Look for conflict markers like:
   ```
   <<<<<<< Updated upstream
   (their changes)
   =======
   (your changes)
   >>>>>>> Stashed changes
   ```
4. Resolve conflicts by choosing what to keep
5. Add the resolved files: `git add filename`
6. The stash pop will be complete

## Quick One-Liner (if you're confident):
```bash
git stash && git pull origin main && git stash pop
```

## What Each Command Does:
- **`git stash`**: Temporarily saves your uncommitted changes
- **`git pull origin main`**: Downloads and merges the latest changes
- **`git stash pop`**: Restores your saved changes and removes them from stash

## After Successfully Pulling:
Don't forget to run Laravel commands if needed:
```bash
# Clear caches
php artisan config:clear
php artisan route:clear
php artisan cache:clear

# Run new migrations
php artisan migrate

# If you have new seeders and want to run them
php artisan db:seed --class=WaterQualityAdminSeeder
```