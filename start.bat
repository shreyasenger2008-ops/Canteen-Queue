@echo off
title CampusDine — Local Dev Servers
color 0A

echo.
echo ================================================================
echo   CampusDine — AI-Based College Canteen Queue Optimization
echo   Starting local development servers...
echo ================================================================
echo.

REM ── Kill any stale processes on ports 8080 and 5173 ──────────────
echo [1/3] Clearing ports 8080 and 5173...
for /f "tokens=5" %%a in ('netstat -ano ^| findstr ":8080 " 2^>nul') do (
    taskkill /PID %%a /F >nul 2>&1
)
for /f "tokens=5" %%a in ('netstat -ano ^| findstr ":5173 " 2^>nul') do (
    taskkill /PID %%a /F >nul 2>&1
)
timeout /t 1 /nobreak >nul

REM ── Start C++ REST API backend ────────────────────────────────────
echo [2/3] Starting C++ REST API on http://localhost:8080 ...
start "CampusDine C++ Backend (port 8080)" /min cmd /c "build\Debug\canteen_app.exe --server 8080"

REM ── Wait for backend to be ready ─────────────────────────────────
echo     Waiting for backend...
:waitBackend
timeout /t 1 /nobreak >nul
curl -s http://127.0.0.1:8080/api/health >nul 2>&1
if errorlevel 1 goto waitBackend
echo     Backend is healthy!

REM ── Start Vite dev server ─────────────────────────────────────────
echo [3/3] Starting React frontend on http://localhost:5173 ...
start "CampusDine Frontend (port 5173)" /min cmd /c "cd frontend && npm run dev"

REM ── Wait for frontend ─────────────────────────────────────────────
echo     Waiting for frontend...
:waitFrontend
timeout /t 1 /nobreak >nul
curl -s http://127.0.0.1:5173 >nul 2>&1
if errorlevel 1 goto waitFrontend
echo     Frontend is ready!

echo.
echo ================================================================
echo   READY — Open in your browser:
echo.
echo   Student Portal  :  http://localhost:5173/
echo   Staff Portal    :  http://localhost:5173/staff
echo   Backend API     :  http://localhost:8080/api/health
echo ================================================================
echo.
echo   Press any key to open the Student Portal in your browser...
pause >nul
start http://localhost:5173/
