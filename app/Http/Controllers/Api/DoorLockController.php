<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use App\Models\DoorLock;
use App\Models\Command;
use App\Http\Resources\DoorLockResource;
use App\Http\Resources\CommandResource;

class DoorLockController extends Controller
{
    public function sendCommand(Request $request)
    {
        $request->validate([
            'door_lock_id' => 'required|exists:door_locks,id',
            'command' => 'required|in:lock,unlock,status',
        ]);

        $doorLock = DoorLock::where('id', $request->door_lock_id)
            ->where('user_id', Auth::id())
            ->firstOrFail();

        $command = Command::create([
            'door_lock_id' => $doorLock->id,
            'command' => $request->command,
            'executed' => false,
        ]);

        $doorLock->last_command = $request->command;
        $doorLock->save();

        return new CommandResource($command);
    }

    public function getPendingCommand(Request $request)
    {

        $request->validate([
            'door_lock_id' => 'required|exists:door_locks,id',
        ]);

        $command = Command::where('door_lock_id', $request->door_lock_id)
            ->where('executed', false)
            ->latest()
            ->first();

        if ($command) {
            return response()->json([
                'command' => $command->command,
                'door_lock_id' => $command->door_lock_id,
                'id' => $command->id,
            ]);
        } else {
            return response()->json(['message' => 'No pending command'], 404);
        }
    }

    public function updateStatus(Request $request)
    {
        $request->validate([
            'door_lock_id' => 'required|exists:door_locks,id',
            'status' => 'nullable|in:locked,unlocked',
            'command_id' => 'nullable|exists:commands,id',
        ]);

        $doorLock = DoorLock::findOrFail($request->door_lock_id);
        if ($request->status) {
            $doorLock->status = $request->status;
            $doorLock->save();
        }

        if ($request->command_id) {
            $command = Command::find($request->command_id);
            if ($command) {
                $command->executed = true;
                $command->save();
            }
        }

        return new DoorLockResource($doorLock);
    }

    public function getStatus(Request $request)
    {
        $request->validate([
            'door_lock_id' => 'required|exists:door_locks,id',
        ]);

        $doorLock = DoorLock::findOrFail($request->door_lock_id);
        return new DoorLockResource($doorLock);
    }

    public function validateRfid(Request $request)
    {
        $request->validate([
            'door_lock_id' => 'required|exists:door_locks,id',
            'uid' => 'required|string',
        ]);

        $doorLock = DoorLock::findOrFail($request->door_lock_id);
        $user = $doorLock->user;

        $isAuthorized = $user->rfidCards()->where('uid', $request->uid)->exists();

        return response()->json(['authorized' => $isAuthorized]);
    }
}
