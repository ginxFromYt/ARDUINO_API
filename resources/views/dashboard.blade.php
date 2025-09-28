<x-layouts.app :title="__('Dashboard')">
    <div class="flex h-full w-full flex-1 flex-col gap-4 rounded-xl">
        <div class="grid auto-rows-min gap-4 md:grid-cols-3">
            <div class="relative aspect-video overflow-hidden rounded-xl border border-neutral-200 dark:border-neutral-700">
                <x-placeholder-pattern class="absolute inset-0 size-full stroke-gray-900/20 dark:stroke-neutral-100/20" />
            </div>
            <div class="relative aspect-video overflow-hidden rounded-xl border border-neutral-200 dark:border-neutral-700">
                <x-placeholder-pattern class="absolute inset-0 size-full stroke-gray-900/20 dark:stroke-neutral-100/20" />
            </div>
            <div class="relative aspect-video overflow-hidden rounded-xl border border-neutral-200 dark:border-neutral-700">
                <x-placeholder-pattern class="absolute inset-0 size-full stroke-gray-900/20 dark:stroke-neutral-100/20" />
            </div>
        </div>
        <div class="relative h-full flex-1 overflow-hidden rounded-xl border border-neutral-200 dark:border-neutral-700 p-6">
            <h2 class="text-xl font-bold mb-4">ESP32 Control Panel</h2>
            <form method="POST" action="{{ route('esp32.sendCommand') }}" class="mb-6">
                @csrf
                <div class="flex gap-2 items-center">
                    <label for="command" class="font-semibold">Command:</label>
                    <select name="command" id="command" class="border rounded px-2 py-1">
                        <option value="lock">Lock</option>
                        <option value="unlock">Unlock</option>
                        <option value="status">Get Status</option>
                    </select>
                    <button type="submit" class="bg-blue-600 text-white px-4 py-2 rounded">Send</button>
                </div>
            </form>
            <div class="mt-4">
                <h3 class="font-semibold mb-2">ESP32 Status:</h3>
                <div class="border rounded p-3 bg-gray-50 dark:bg-gray-800">
                    <!-- Later: Show status fetched from API -->
                    <span class="text-gray-700 dark:text-gray-200">Status will appear here.</span>
                </div>
            </div>
        </div>
    </div>
</x-layouts.app>
