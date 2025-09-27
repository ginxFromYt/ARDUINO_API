<?php

namespace App\Http\Resources;

use Illuminate\Http\Resources\Json\JsonResource;

class CommandResource extends JsonResource
{
    /**
     * Transform the resource into an array.
     *
     * @param  \Illuminate\Http\Request  $request
     * @return array<string, mixed>
     */
    public function toArray($request)
    {
        return [
            'id' => $this->id,
            'door_lock_id' => $this->door_lock_id,
            'command' => $this->command,
            'executed' => $this->executed,
            'created_at' => $this->created_at,
        ];
    }
}
