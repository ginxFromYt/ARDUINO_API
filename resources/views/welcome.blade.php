
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Smart Door Lock System API</title>
    <link rel="icon" href="/favicon.ico" sizes="any">
    <link rel="icon" href="/favicon.svg" type="image/svg+xml">
    <link rel="apple-touch-icon" href="/apple-touch-icon.png">
    <style>
        body { 
            font-family: Arial, sans-serif; 
            background: #f8fafc; 
            color: #222; 
            text-align: center; 
            padding: 20px; 
        }
        .container { 
            background: #fff; 
            border-radius: 8px; 
            box-shadow: 0 2px 8px #eee; 
            display: inline-block; 
            padding: 24px 32px; 
            max-width: 90%; 
            box-sizing: border-box; 
        }
        h1 { color: #2563eb; }
        a { color: #2563eb; text-decoration: none; }
        .api-list { 
            text-align: left; 
            margin: 24px auto; 
            max-width: 400px; 
            padding: 0; 
        }
        .api-list li { 
            margin-bottom: 8px; 
            font-size: 0.9em; 
        }
        @media (min-width: 768px) {
            body { padding: 40px; }
            .container { padding: 32px 48px; max-width: none; }
            .api-list li { font-size: 1em; }
        }
    </style>
</head>
<body>
    
    <div class="container">
        <header class="w-full lg:max-w-4xl max-w-[335px] text-sm mb-6 not-has-[nav]:hidden">
            @if (Route::has('login'))
                <nav class="flex items-center justify-end gap-4">
                    @auth
                        <a
                            href="{{ url('/dashboard') }}"
                            class="inline-block px-5 py-1.5 dark:text-[#EDEDEC] border-[#19140035] hover:border-[#1915014a] border text-[#1b1b18] dark:border-[#3E3E3A] dark:hover:border-[#62605b] rounded-sm text-sm leading-normal"
                        >
                            Dashboard
                        </a>
                    @else
                        <a
                            href="{{ route('login') }}"
                            class="inline-block px-5 py-1.5 dark:text-[#EDEDEC] text-[#1b1b18] border border-transparent hover:border-[#19140035] dark:hover:border-[#3E3E3A] rounded-sm text-sm leading-normal"
                        >
                            Log in
                        </a>

                        @if (Route::has('register'))
                            <a
                                href="{{ route('register') }}"
                                class="inline-block px-5 py-1.5 dark:text-[#EDEDEC] border-[#19140035] hover:border-[#1915014a] border text-[#1b1b18] dark:border-[#3E3E3A] dark:hover:border-[#62605b] rounded-sm text-sm leading-normal">
                                Register
                            </a>
                        @endif
                    @endauth
                </nav>
            @endif
        </header>
        

        @if (Route::has('login'))
            <div class="h-14.5 hidden lg:block"></div>
        @endif
        <h1>Smart Door Lock System API</h1>
        <p>Welcome to the backend for your Smart Door Lock System.</p>
        <p>This API powers secure access, remote control, and real-time status for your smart locks.</p>
        <h2>Key API Endpoints</h2>
        <ul class="api-list">
            <li><strong>POST</strong> <code>/api/login</code> &mdash; Authenticate and receive API token</li>
            <li><strong>POST</strong> <code>/api/door/command</code> &mdash; Send lock/unlock command</li>
            <li><strong>GET</strong> <code>/api/door/command</code> &mdash; ESP32 fetches latest pending command</li>
            <li><strong>POST</strong> <code>/api/door/status</code> &mdash; ESP32 updates current status</li>
            <li><strong>GET</strong> <code>/api/door/status</code> &mdash; User fetches latest door status</li>
        </ul>
        <p>For documentation and source code, visit:
            <a href="https://github.com/ginxFromYt/ARDUINO_API" target="_blank">GitHub Repository</a>
        </p>
        <p style="color: #888; font-size: 0.95em;">&copy; 2025 Smart Door Lock System</p>
    </div>
</body>
</html>


  
</html>
