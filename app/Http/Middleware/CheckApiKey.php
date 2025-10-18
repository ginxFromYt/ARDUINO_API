<?php

namespace App\Http\Middleware;

use Closure;
use Illuminate\Http\Request;
use Symfony\Component\HttpFoundation\Response;

class CheckApiKey
{
    /**
     * Handle an incoming request.
     *
     * @param  \Closure(\Illuminate\Http\Request): (\Symfony\Component\HttpFoundation\Response)  $next
     */
    public function handle(Request $request, Closure $next): Response
    {
        // Get API key from request headers (supports multiple header formats)
        $apiKey = $request->header('x-api-key') 
               ?? $request->header('api-key') 
               ?? $request->header('authorization');

        // Remove 'Bearer ' prefix if present
        if ($apiKey && str_starts_with(strtolower($apiKey), 'bearer ')) {
            $apiKey = substr($apiKey, 7);
        }

        // Get the expected API key from environment
        $expectedApiKey = config('app.api_key');

        // Check if API key is valid
        if (!$apiKey || !$expectedApiKey || !hash_equals($expectedApiKey, $apiKey)) {
            return response()->json([
                'success' => false,
                'message' => 'Unauthorized - Invalid or missing API key',
                'error' => 'INVALID_API_KEY'
            ], 401);
        }

        return $next($request);
    }
}
