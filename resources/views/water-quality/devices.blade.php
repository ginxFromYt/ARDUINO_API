<x-layouts.app :title="__('Water Quality Devices')">
    <div class="flex h-full w-full flex-1 flex-col gap-4">
        <!-- Header Section -->
        <div class="bg-white dark:bg-gray-800 rounded-xl border border-neutral-200 dark:border-neutral-700 p-6">
            <div class="flex justify-between items-center">
                <div>
                    <h1 class="text-2xl font-bold text-gray-900 dark:text-white">Water Quality Devices</h1>
                    <p class="text-gray-600 dark:text-gray-400 mt-1">Manage and monitor all water quality monitoring devices</p>
                </div>
                <div class="flex gap-2">
                    <a href="{{ route('water-quality.dashboard') }}" class="bg-gray-600 hover:bg-gray-700 text-white px-4 py-2 rounded-lg text-sm font-medium">
                        Back to Dashboard
                    </a>
                    <button onclick="refreshData()" class="bg-green-600 hover:bg-green-700 text-white px-4 py-2 rounded-lg text-sm font-medium">
                        Refresh
                    </button>
                </div>
            </div>
        </div>

        <!-- Devices List -->
        <div class="bg-white dark:bg-gray-800 rounded-xl border border-neutral-200 dark:border-neutral-700 p-6">
            @if($devices->count() > 0)
                <div class="grid grid-cols-1 lg:grid-cols-2 xl:grid-cols-3 gap-6">
                    @foreach($devices as $device)
                        <div class="border border-gray-200 dark:border-gray-700 rounded-lg p-6 hover:shadow-lg transition-shadow">
                            <!-- Device Header -->
                            <div class="flex justify-between items-start mb-4">
                                <div>
                                    <h3 class="text-lg font-semibold text-gray-900 dark:text-white">{{ $device->device_id }}</h3>
                                    <p class="text-sm text-gray-500 dark:text-gray-400">{{ $device->location ?? 'No location set' }}</p>
                                </div>
                                <div class="flex items-center">
                                    @if($device->is_online)
                                        <div class="flex items-center text-green-600 dark:text-green-400">
                                            <div class="w-2 h-2 bg-green-500 rounded-full mr-2 animate-pulse"></div>
                                            <span class="text-xs font-medium">Online</span>
                                        </div>
                                    @else
                                        <div class="flex items-center text-red-600 dark:text-red-400">
                                            <div class="w-2 h-2 bg-red-500 rounded-full mr-2"></div>
                                            <span class="text-xs font-medium">Offline</span>
                                        </div>
                                    @endif
                                </div>
                            </div>

                            <!-- Device Stats -->
                            <div class="space-y-3 mb-4">
                                <div class="flex justify-between items-center">
                                    <span class="text-sm text-gray-600 dark:text-gray-400">Total Readings:</span>
                                    <span class="text-sm font-medium text-gray-900 dark:text-white">{{ number_format($device->total_readings) }}</span>
                                </div>
                                <div class="flex justify-between items-center">
                                    <span class="text-sm text-gray-600 dark:text-gray-400">Alerts:</span>
                                    <span class="text-sm font-medium {{ $device->alert_count > 0 ? 'text-red-600 dark:text-red-400' : 'text-gray-900 dark:text-white' }}">
                                        {{ $device->alert_count }}
                                    </span>
                                </div>
                                <div class="flex justify-between items-center">
                                    <span class="text-sm text-gray-600 dark:text-gray-400">Last Seen:</span>
                                    <span class="text-sm font-medium text-gray-900 dark:text-white">{{ \Carbon\Carbon::parse($device->last_seen)->diffForHumans() }}</span>
                                </div>
                            </div>

                            <!-- Latest Reading -->
                            @if($device->latest_reading)
                                <div class="border-t border-gray-200 dark:border-gray-700 pt-4 space-y-2">
                                    <h4 class="text-sm font-medium text-gray-900 dark:text-white mb-2">Latest Reading:</h4>
                                    <div class="grid grid-cols-3 gap-2 text-xs">
                                        <div class="text-center">
                                            <div class="text-gray-600 dark:text-gray-400">Turbidity</div>
                                            <div class="font-medium text-gray-900 dark:text-white">{{ number_format($device->latest_reading->turbidity, 2) }}V</div>
                                        </div>
                                        <div class="text-center">
                                            <div class="text-gray-600 dark:text-gray-400">TDS</div>
                                            <div class="font-medium text-gray-900 dark:text-white">{{ number_format($device->latest_reading->tds, 1) }} ppm</div>
                                        </div>
                                        <div class="text-center">
                                            <div class="text-gray-600 dark:text-gray-400">pH</div>
                                            <div class="font-medium text-gray-900 dark:text-white">{{ number_format($device->latest_reading->ph, 2) }}</div>
                                        </div>
                                    </div>
                                    
                                    <!-- Alert Status -->
                                    <div class="flex justify-center mt-3">
                                        @if($device->latest_reading->alert_level === 'normal')
                                            <span class="bg-green-100 text-green-800 text-xs font-medium px-3 py-1 rounded-full dark:bg-green-900 dark:text-green-300">Normal</span>
                                        @elseif($device->latest_reading->alert_level === 'warning')
                                            <span class="bg-yellow-100 text-yellow-800 text-xs font-medium px-3 py-1 rounded-full dark:bg-yellow-900 dark:text-yellow-300">Warning</span>
                                        @else
                                            <span class="bg-red-100 text-red-800 text-xs font-medium px-3 py-1 rounded-full dark:bg-red-900 dark:text-red-300">Critical</span>
                                        @endif
                                    </div>
                                </div>
                            @endif

                            <!-- Action Buttons -->
                            <div class="mt-6 flex gap-2">
                                <a href="{{ route('water-quality.device', $device->device_id) }}" class="flex-1 bg-blue-600 hover:bg-blue-700 text-white text-center py-2 px-3 rounded text-sm font-medium">
                                    View Details
                                </a>
                                <a href="{{ route('water-quality.export', $device->device_id) }}" class="bg-gray-600 hover:bg-gray-700 text-white py-2 px-3 rounded text-sm font-medium">
                                    Export
                                </a>
                            </div>
                        </div>
                    @endforeach
                </div>
            @else
                <div class="text-center py-12">
                    <svg class="mx-auto h-16 w-16 text-gray-400" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 19v-6a2 2 0 00-2-2H5a2 2 0 00-2 2v6a2 2 0 002 2h2a2 2 0 002-2zm0 0V9a2 2 0 012-2h2a2 2 0 012 2v10m-6 0a2 2 0 002 2h2a2 2 0 002-2m0 0V5a2 2 0 012-2h2a2 2 0 012 2v14a2 2 0 01-2 2h-2a2 2 0 01-2-2z"/>
                    </svg>
                    <h3 class="mt-4 text-lg font-medium text-gray-900 dark:text-white">No devices found</h3>
                    <p class="mt-2 text-sm text-gray-500 dark:text-gray-400">
                        No water quality monitoring devices have sent data yet. Make sure your Arduino devices are properly configured and connected.
                    </p>
                    <div class="mt-6">
                        <a href="{{ route('water-quality.dashboard') }}" class="bg-blue-600 hover:bg-blue-700 text-white px-4 py-2 rounded-lg text-sm font-medium">
                            Back to Dashboard
                        </a>
                    </div>
                </div>
            @endif
        </div>
    </div>

    @push('scripts')
    <script>
        function refreshData() {
            window.location.reload();
        }

        // Auto-refresh every 2 minutes
        setInterval(function() {
            if (document.visibilityState === 'visible') {
                refreshData();
            }
        }, 120000);
    </script>
    @endpush
</x-layouts.app>