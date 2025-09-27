<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\Factories\HasFactory;

class DoorLock extends Model
{
    use HasFactory;

    protected $fillable = [
        'user_id',
        'status',
        'last_command',
    ];

    public function user()
    {
        return $this->belongsTo(User::class);
    }

    public function commands()
    {
        return $this->hasMany(Command::class);
    }
}
