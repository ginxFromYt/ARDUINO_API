<?php

namespace App\Http\Middleware;

use Closure;
use Illuminate\Http\Request;
use Symfony\Component\HttpFoundation\Response;

class CheckUserRole
{
    /**
     * Handle an incoming request.
     *
     * @param  \Closure(\Illuminate\Http\Request): (\Symfony\Component\HttpFoundation\Response)  $next
     * @param  string  $role  The required role (water-quality-admin, door-lock-admin, admin)
     */
    public function handle(Request $request, Closure $next, string $role = null): Response
    {
        $user = $request->user();

        if (!$user) {
            return redirect()->route('login');
        }

        // Define role checks
        $roles = [
            'water-quality-admin' => function ($user) {
                return $user->email === 'wqmadmin@mail.com' || $user->is_admin;
            },
            'door-lock-admin' => function ($user) {
                return $user->email === 'homeadmin@mail.com' || $user->is_admin;
            },
            'admin' => function ($user) {
                return $user->is_admin;
            },
        ];

        // Check if user has the required role
        if ($role && isset($roles[$role])) {
            if (!$roles[$role]($user)) {
                abort(403, 'Access denied. You do not have permission to access this area.');
            }
        }

        return $next($request);
    }
}
