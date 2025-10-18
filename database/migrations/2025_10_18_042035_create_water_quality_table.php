<?php

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;

return new class extends Migration
{
    /**
     * Run the migrations.
     */
    public function up(): void
    {
        Schema::create('water_quality', function (Blueprint $table) {
            $table->id();
            $table->string('device_id')->index(); // Arduino device identifier
            $table->decimal('turbidity', 8, 3); // Turbidity voltage reading
            $table->decimal('tds', 8, 2); // TDS in ppm
            $table->decimal('ph', 4, 2); // pH value
            $table->enum('status', ['normal', 'warning', 'critical'])->default('normal');
            $table->enum('alert_level', ['normal', 'warning', 'critical'])->default('normal');
            $table->string('location')->nullable(); // Device location description
            
            // Raw sensor voltages for debugging
            $table->decimal('raw_turbidity_voltage', 6, 3)->nullable();
            $table->decimal('raw_tds_voltage', 6, 3)->nullable();
            $table->decimal('raw_ph_voltage', 6, 3)->nullable();
            
            // Additional metadata
            $table->json('sensor_states')->nullable(); // Store alert states from Arduino
            $table->text('alert_message')->nullable(); // Store alert descriptions
            
            $table->timestamps();
            
            // Indexes for common queries
            $table->index(['device_id', 'created_at']);
            $table->index(['alert_level', 'created_at']);
            $table->index('created_at');
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('water_quality');
    }
};
