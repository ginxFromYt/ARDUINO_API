<x-layouts.app :title="__('Device: ' . $deviceId)">
    <div class="flex h-full w-full flex-1 flex-col gap-4">
        <!-- Header Section -->
        <div class="bg-white dark:bg-gray-800 rounded-xl border border-neutral-200 dark:border-neutral-700 p-6">
            <div class="flex justify-between items-start">
                <div>
                    <h1 class="text-2xl font-bold text-gray-900 dark:text-white">{{ $deviceId }}</h1>
                    <p class="text-gray-600 dark:text-gray-400 mt-1">{{ $deviceStats['latest_reading']->location ?? 'No location set' }}</p>
                </div>
                <div class="flex gap-2">
                    <select id="timeRange" onchange="changeTimeRange()" class="border border-gray-300 dark:border-gray-600 rounded-lg px-3 py-2 text-sm bg-white dark:bg-gray-700 text-gray-900 dark:text-white">
                        <option value="1" {{ $hours == 1 ? 'selected' : '' }}>Last Hour</option>
                        <option value="6" {{ $hours == 6 ? 'selected' : '' }}>Last 6 Hours</option>
                        <option value="24" {{ $hours == 24 ? 'selected' : '' }}>Last 24 Hours</option>
                        <option value="72" {{ $hours == 72 ? 'selected' : '' }}>Last 3 Days</option>
                        <option value="168" {{ $hours == 168 ? 'selected' : '' }}>Last Week</option>
                    </select>
                    <a href="{{ route('water-quality.export', $deviceId) }}?hours={{ $hours }}" class="bg-green-600 hover:bg-green-700 text-white px-4 py-2 rounded-lg text-sm font-medium">
                        Export CSV
                    </a>
                    <a href="{{ route('water-quality.devices') }}" class="bg-gray-600 hover:bg-gray-700 text-white px-4 py-2 rounded-lg text-sm font-medium">
                        Back to Devices
                    </a>
                </div>
            </div>
        </div>

        <!-- Device Stats -->
        <div class="grid grid-cols-1 md:grid-cols-4 gap-4">
            <div class="bg-white dark:bg-gray-800 rounded-xl border border-neutral-200 dark:border-neutral-700 p-4">
                <div class="flex items-center">
                    <div class="p-2 bg-blue-600 rounded-lg">
                        <svg class="w-5 h-5 text-white" fill="currentColor" viewBox="0 0 20 20">
                            <path d="M9 12l2 2 4-4m6 2a9 9 0 11-18 0 9 9 0 0118 0z"/>
                        </svg>
                    </div>
                    <div class="ml-4">
                        <p class="text-sm font-medium text-gray-500 dark:text-gray-400">Total Readings</p>
                        <p class="text-xl font-bold text-gray-900 dark:text-white">{{ number_format($deviceStats['total_readings']) }}</p>
                    </div>
                </div>
            </div>

            <div class="bg-white dark:bg-gray-800 rounded-xl border border-neutral-200 dark:border-neutral-700 p-4">
                <div class="flex items-center">
                    <div class="p-2 bg-yellow-600 rounded-lg">
                        <svg class="w-5 h-5 text-white" fill="currentColor" viewBox="0 0 20 20">
                            <path fill-rule="evenodd" d="M8.257 3.099c.765-1.36 2.722-1.36 3.486 0l5.58 9.92c.75 1.334-.213 2.98-1.742 2.98H4.42c-1.53 0-2.493-1.646-1.743-2.98l5.58-9.92zM11 13a1 1 0 11-2 0 1 1 0 012 0zm-1-8a1 1 0 00-1 1v3a1 1 0 002 0V6a1 1 0 00-1-1z"/>
                        </svg>
                    </div>
                    <div class="ml-4">
                        <p class="text-sm font-medium text-gray-500 dark:text-gray-400">Total Alerts</p>
                        <p class="text-xl font-bold text-gray-900 dark:text-white">{{ $deviceStats['alerts_count'] }}</p>
                    </div>
                </div>
            </div>

            <div class="bg-white dark:bg-gray-800 rounded-xl border border-neutral-200 dark:border-neutral-700 p-4">
                <div class="flex items-center">
                    <div class="p-2 {{ $deviceStats['last_online'] && $deviceStats['last_online']->isAfter(now()->subMinutes(10)) ? 'bg-green-600' : 'bg-red-600' }} rounded-lg">
                        <svg class="w-5 h-5 text-white" fill="currentColor" viewBox="0 0 20 20">
                            <path fill-rule="evenodd" d="M10 18a8 8 0 100-16 8 8 0 000 16zm3.707-9.293a1 1 0 00-1.414-1.414L9 10.586 7.707 9.293a1 1 0 00-1.414 1.414l2 2a1 1 0 001.414 0l4-4z"/>
                        </svg>
                    </div>
                    <div class="ml-4">
                        <p class="text-sm font-medium text-gray-500 dark:text-gray-400">Status</p>
                        <p class="text-xl font-bold text-gray-900 dark:text-white">
                            {{ $deviceStats['last_online'] && $deviceStats['last_online']->isAfter(now()->subMinutes(10)) ? 'Online' : 'Offline' }}
                        </p>
                    </div>
                </div>
            </div>

            <div class="bg-white dark:bg-gray-800 rounded-xl border border-neutral-200 dark:border-neutral-700 p-4">
                <div class="flex items-center">
                    <div class="p-2 bg-purple-600 rounded-lg">
                        <svg class="w-5 h-5 text-white" fill="currentColor" viewBox="0 0 20 20">
                            <path d="M10 2L3 7v11c0 1.1.9 2 2 2h10c1.1 0 2-.9 2-2V7l-7-5z"/>
                        </svg>
                    </div>
                    <div class="ml-4">
                        <p class="text-sm font-medium text-gray-500 dark:text-gray-400">Last Seen</p>
                        <p class="text-lg font-bold text-gray-900 dark:text-white">{{ $deviceStats['last_online'] ? $deviceStats['last_online']->diffForHumans() : 'Never' }}</p>
                    </div>
                </div>
            </div>
        </div>

        <!-- Latest Reading Card -->
        @if($deviceStats['latest_reading'])
        <div class="bg-white dark:bg-gray-800 rounded-xl border border-neutral-200 dark:border-neutral-700 p-6">
            <h2 class="text-xl font-semibold text-gray-900 dark:text-white mb-4">Latest Reading</h2>
            <div class="grid grid-cols-1 md:grid-cols-3 gap-6">
                <div class="text-center">
                    <div class="text-3xl font-bold text-blue-600 dark:text-blue-400">{{ number_format($deviceStats['latest_reading']->turbidity, 2) }}V</div>
                    <div class="text-sm text-gray-600 dark:text-gray-400 mt-1">Turbidity</div>
                    <div class="text-xs text-gray-500 dark:text-gray-500 mt-1">{{ $deviceStats['latest_reading']->turbidity < 2.32 ? 'Turbid Water' : 'Clear Water' }}</div>
                </div>
                <div class="text-center">
                    <div class="text-3xl font-bold text-green-600 dark:text-green-400">{{ number_format($deviceStats['latest_reading']->tds, 1) }}</div>
                    <div class="text-sm text-gray-600 dark:text-gray-400 mt-1">TDS (ppm)</div>
                    <div class="text-xs text-gray-500 dark:text-gray-500 mt-1">{{ $deviceStats['latest_reading']->tds > 500 ? 'High TDS' : 'Normal TDS' }}</div>
                </div>
                <div class="text-center">
                    <div class="text-3xl font-bold text-red-600 dark:text-red-400">{{ number_format($deviceStats['latest_reading']->ph, 2) }}</div>
                    <div class="text-sm text-gray-600 dark:text-gray-400 mt-1">pH Level</div>
                    <div class="text-xs text-gray-500 dark:text-gray-500 mt-1">
                        {{ ($deviceStats['latest_reading']->ph >= 6.5 && $deviceStats['latest_reading']->ph <= 8.5) ? 'Safe Range' : 'Out of Range' }}
                    </div>
                </div>
            </div>
            <div class="flex justify-center mt-4">
                @if($deviceStats['latest_reading']->alert_level === 'normal')
                    <span class="bg-green-100 text-green-800 text-sm font-medium px-4 py-2 rounded-full dark:bg-green-900 dark:text-green-300">All Parameters Normal</span>
                @elseif($deviceStats['latest_reading']->alert_level === 'warning')
                    <span class="bg-yellow-100 text-yellow-800 text-sm font-medium px-4 py-2 rounded-full dark:bg-yellow-900 dark:text-yellow-300">Warning: {{ $deviceStats['latest_reading']->status_description }}</span>
                @else
                    <span class="bg-red-100 text-red-800 text-sm font-medium px-4 py-2 rounded-full dark:bg-red-900 dark:text-red-300">Critical: {{ $deviceStats['latest_reading']->status_description }}</span>
                @endif
            </div>
        </div>
        @endif

        <!-- Chart Section -->
        <div class="bg-white dark:bg-gray-800 rounded-xl border border-neutral-200 dark:border-neutral-700 p-6">
            <h2 class="text-xl font-semibold text-gray-900 dark:text-white mb-4">Sensor Data Trends (Last {{ $hours }} {{ $hours == 1 ? 'Hour' : 'Hours' }})</h2>
            <div class="h-96">
                <canvas id="sensorChart" width="400" height="200"></canvas>
            </div>
        </div>

        <!-- Recent Readings Table -->
        <div class="bg-white dark:bg-gray-800 rounded-xl border border-neutral-200 dark:border-neutral-700 p-6">
            <h2 class="text-xl font-semibold text-gray-900 dark:text-white mb-4">Recent Readings</h2>
            
            @if($readings->count() > 0)
                <div class="overflow-x-auto">
                    <table class="min-w-full divide-y divide-gray-200 dark:divide-gray-700">
                        <thead class="bg-gray-50 dark:bg-gray-700">
                            <tr>
                                <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-300 uppercase tracking-wider">Timestamp</th>
                                <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-300 uppercase tracking-wider">Turbidity (V)</th>
                                <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-300 uppercase tracking-wider">TDS (ppm)</th>
                                <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-300 uppercase tracking-wider">pH</th>
                                <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-300 uppercase tracking-wider">Status</th>
                            </tr>
                        </thead>
                        <tbody class="bg-white dark:bg-gray-800 divide-y divide-gray-200 dark:divide-gray-700">
                            @foreach($readings as $reading)
                            <tr class="hover:bg-gray-50 dark:hover:bg-gray-700">
                                <td class="px-6 py-4 whitespace-nowrap text-sm text-gray-900 dark:text-white">
                                    {{ $reading->created_at->format('M j, Y H:i:s') }}
                                </td>
                                <td class="px-6 py-4 whitespace-nowrap text-sm text-gray-900 dark:text-white">
                                    {{ number_format($reading->turbidity, 2) }}
                                </td>
                                <td class="px-6 py-4 whitespace-nowrap text-sm text-gray-900 dark:text-white">
                                    {{ number_format($reading->tds, 1) }}
                                </td>
                                <td class="px-6 py-4 whitespace-nowrap text-sm text-gray-900 dark:text-white">
                                    {{ number_format($reading->ph, 2) }}
                                </td>
                                <td class="px-6 py-4 whitespace-nowrap">
                                    @if($reading->alert_level === 'normal')
                                        <span class="bg-green-100 text-green-800 text-xs font-medium px-2.5 py-0.5 rounded-full dark:bg-green-900 dark:text-green-300">Normal</span>
                                    @elseif($reading->alert_level === 'warning')
                                        <span class="bg-yellow-100 text-yellow-800 text-xs font-medium px-2.5 py-0.5 rounded-full dark:bg-yellow-900 dark:text-yellow-300">Warning</span>
                                    @else
                                        <span class="bg-red-100 text-red-800 text-xs font-medium px-2.5 py-0.5 rounded-full dark:bg-red-900 dark:text-red-300">Critical</span>
                                    @endif
                                </td>
                            </tr>
                            @endforeach
                        </tbody>
                    </table>
                </div>
                
                <div class="mt-6">
                    {{ $readings->appends(['hours' => $hours])->links() }}
                </div>
            @else
                <div class="text-center py-8">
                    <p class="text-gray-500 dark:text-gray-400">No readings found for the selected time period.</p>
                </div>
            @endif
        </div>
    </div>

    @push('scripts')
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script>
        function changeTimeRange() {
            const hours = document.getElementById('timeRange').value;
            window.location.href = `{{ route('water-quality.device', $deviceId) }}?hours=${hours}`;
        }

        // Load chart data
        fetch(`{{ route('water-quality.chart-data', $deviceId) }}?hours={{ $hours }}`)
            .then(response => response.json())
            .then(data => {
                const ctx = document.getElementById('sensorChart').getContext('2d');
                new Chart(ctx, {
                    type: 'line',
                    data: data,
                    options: {
                        responsive: true,
                        maintainAspectRatio: false,
                        scales: {
                            x: {
                                display: true,
                                title: {
                                    display: true,
                                    text: 'Time'
                                }
                            },
                            y: {
                                type: 'linear',
                                display: true,
                                position: 'left',
                                title: {
                                    display: true,
                                    text: 'Turbidity (V)'
                                }
                            },
                            y1: {
                                type: 'linear',
                                display: true,
                                position: 'right',
                                title: {
                                    display: true,
                                    text: 'TDS (ppm)'
                                },
                                grid: {
                                    drawOnChartArea: false,
                                }
                            },
                            y2: {
                                type: 'linear',
                                display: false,
                                title: {
                                    display: true,
                                    text: 'pH'
                                }
                            }
                        },
                        plugins: {
                            legend: {
                                display: true,
                                position: 'top'
                            },
                            tooltip: {
                                mode: 'index',
                                intersect: false
                            }
                        },
                        interaction: {
                            mode: 'nearest',
                            axis: 'x',
                            intersect: false
                        }
                    }
                });
            })
            .catch(error => console.error('Error loading chart data:', error));
    </script>
    @endpush
</x-layouts.app>