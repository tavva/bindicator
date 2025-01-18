#include "setup_server.h"
#include <WiFi.h>
#include "calendar_handler.h"

const char* SetupServer::getSetupPage() {
    static String page;

    String wifiStatus;
    String sectionClass;

    if (config.wifi_ssid.isEmpty()) {
        wifiStatus = "<span class='status-incomplete'>Not Configured</span>";
        sectionClass = " incomplete";
    } else if (WiFi.status() != WL_CONNECTED) {
        wifiStatus = "<span class='status-incomplete'>Configured (" + config.wifi_ssid + ") - Not Connected</span>";
        sectionClass = " incomplete";
    } else {
        wifiStatus = "<span class='status-complete'>Configured (" + config.wifi_ssid + ") - Connected</span>";
        sectionClass = " complete";
    }

    String oauthStatus = oauthHandler.isAuthorized() ?
        "<span class='status-complete'>Connected</span>" :
        "<span class='status-incomplete'>Not Connected</span>";

    String calendarStatus = "<span class='status-complete' id='calendar-status'>Loading calendar name...</span>";

    page =
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
            "<title>Bindicator Setup</title>"
            "<style>"
                "body { font-family: Arial, sans-serif; max-width: 600px; margin: 0 auto; padding: 20px; background-color: #1e1e1e; color: #ffffff; }"
                "@media (max-width: 640px) { body { padding: 10px; } }"

                ".setup-section { display: flex; flex-direction: column; margin-bottom: 30px; padding: 20px; border: 1px solid #333; border-radius: 5px; position: relative; background-color: #252525; }"
                "@media (max-width: 640px) { .setup-section { padding: 15px; margin-bottom: 20px; } }"

                "h1 { color: #ffffff; }"
                "@media (max-width: 640px) { h1 { font-size: 24px; margin: 10px 0; } }"

                "h2 { color: #ffffff; margin-top: 0; display: flex; align-items: center; gap: 10px; }"
                "@media (max-width: 640px) { h2 { font-size: 20px; } }"

                ".step-number { background-color: #444; color: white; width: 24px; height: 24px; border-radius: 12px; display: flex; align-items: center; justify-content: center; font-size: 14px; }"
                "@media (max-width: 640px) { .step-number { width: 20px; height: 20px; font-size: 12px; } }"

                "label { display: block; margin-bottom: 5px; }"

                "input[type=\"text\"], input[type=\"password\"], select { width: 100%; box-sizing: border-box; padding: 8px; margin-bottom: 10px; border: 1px solid #444; border-radius: 4px; background-color: #333; color: #ffffff; }"
                "@media (max-width: 640px) { input[type=\"text\"], input[type=\"password\"], select { padding: 10px; } }"

                "button { background-color: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; }"
                "@media (max-width: 640px) { "
                    "button { width: 100%; padding: 12px; margin-bottom: 8px; } "
                    ".setup-section div[style*=\"display: flex\"] { flex-direction: column; }"
                    ".setup-section div[style*=\"display: flex\"] button { margin-right: 0; }"
                "}"

                ".setup-complete { margin: 30px 0; padding: 30px; border: 2px solid #4CAF50; border-radius: 10px; background-color: #252525; text-align: center; }"
                "@media (max-width: 640px) { .setup-complete { padding: 20px; margin: 20px 0; } }"

                ".launch-button { background-color: #4CAF50; color: white; padding: 15px 30px; border: none; border-radius: 5px; font-size: 18px; cursor: pointer; margin-top: 20px; }"
                "@media (max-width: 640px) { .launch-button { padding: 12px 24px; font-size: 16px; width: 100%; } }"

                "#upcoming-bins-list { color: #888; font-size: 0.9em; margin: 0; padding-left: 20px; }"
                "@media (max-width: 640px) { #upcoming-bins-list { padding-left: 15px; } }"

                ".status-cell { margin-top: 15px; padding-top: 15px; border-top: 1px solid #444; word-break: break-word; }"

                ".setup-section::before { content: ''; position: absolute; top: 0; left: 0; width: 4px; height: 100%; border-radius: 5px 0 0 5px; }"
                ".setup-section.incomplete::before { background-color: #f44336; }"
                ".setup-section.complete::before { background-color: #4CAF50; }"
                ".setup-section.disabled { opacity: 0.7; background-color: #1a1a1a; }"
                ".status { margin-top: 10px; }"
                ".status-complete { color: #4CAF50; }"
                ".status-incomplete { color: #f44336; }"
                ".help-text { font-size: 0.9em; color: #888; margin-top: 5px; }"
            "</style>"
        "</head>"
        "<body>"
            "<h1>Bindicator Setup</h1>";

    // WiFi Section
    page += "<div class='setup-section" + sectionClass + "'>"
            "<div class='section-content'>"
                "<h2><span class='step-number'>1</span> WiFi Setup</h2>"
                "<form action='/save' method='POST'>"
                    "<label for='ssid'>WiFi Name:</label>"
                    "<input type='text' id='ssid' name='ssid' value='" + config.wifi_ssid + "'>"
                    "<label for='password'>WiFi Password:</label>"
                    "<input type='password' id='password' name='password' value='" + config.wifi_password + "'>"
                    "<button type='submit'>Save WiFi Settings</button>"
                "</form>"
            "</div>"
            "<div class='status-cell'>Status: " + wifiStatus + "</div>"
            "</div>";

    // OAuth Section
    String oauthContent = config.wifi_ssid.isEmpty() ?
        "<p>Please complete WiFi setup first</p>" :
        "<p>Click below to connect your Google Calendar:</p><a href='/oauth'><button>Connect Google Calendar</button></a>";

    page += "<div class='setup-section" +
            String(oauthHandler.isAuthorized() ? " complete" : " incomplete") +
            String(config.wifi_ssid.isEmpty() ? " disabled" : "") + "'>" +
            "<div class='section-content'>"
                "<h2><span class='step-number'>2</span> Google Calendar Setup</h2>" +
                oauthContent +
            "</div>"
            "<div class='status-cell'>Status: " + oauthStatus + "</div>"
            "</div>";

    // Setup complete
    if (WiFi.status() == WL_CONNECTED && oauthHandler.isAuthorized()) {
        page += "<div id='setupComplete' class='setup-complete'>"
                "<h2>Your Bindicator is Ready!</h2>"
                "<p>Setup is complete and your Bindicator is configured.</p>"
                "<p><small>If your bin events are not on your main calendar, you can change the calendar in the calendar settings section below.</small></p>"
                "<button class='launch-button' onclick='restartDevice()'>Launch Bindicator</button>"
                "</div>";
    }

    // Calendar Section
    page += "<div id='calendar-section' class='setup-section" +
            String(!oauthHandler.isAuthorized() ? " disabled" : " complete") + "'>" +
            "<div class='section-content'>"
                "<h2><span class='step-number'>3</span> Calendar Settings</h2>"
                "<form action='/save-calendar' method='POST'>"
                    "<label for='calendar_id' style='margin-bottom: 10px; display: block;'>Select Calendar:</label>"
                    "<div id='calendar-select-container'>"
                        "<select id='calendar_id' name='calendar_id' style='display:none;' onchange='updateCalendar(event)'>"
                            "<option value='" + ConfigManager::getCalendarId() + "' selected>" +
                            (ConfigManager::getCalendarId() == "primary" ? "Main Calendar" : ConfigManager::getCalendarId()) +
                            "</option>"
                        "</select>"
                        "<div id='calendar-loading'>Loading calendars...</div>"
                    "</div>"
                    "<p class='help-text'>Choose which calendar contains your bin collection schedule</p>"
                "</form>"
                "<div id='upcoming-bins' style='margin-top: 15px; display: none;'>"
                    "<hr style='border: none; border-top: 1px solid #444; margin: 15px 0;'>"
                    "<h3 style='color: #ffffff; margin: 0 0 10px 0;'>Upcoming Collections</h3>"
                    "<ul id='upcoming-bins-list' style='color: #888; font-size: 0.9em; margin: 0; padding-left: 20px;'>"
                        "<li>Loading...</li>"
                    "</ul>"
                "</div>"
            "</div>"
            "<div class='status-cell'>Status: " + calendarStatus + "</div>"
            "</div>";

    // Device Control Section
    page += "<div class='setup-section'>"
            "<h2 style='color: #ff4444;'>Device Control</h2>"
            "<div style='display: flex; gap: 10px;'>"
                "<button onclick='restartDevice()' style='background-color: #ff4444;'>Restart Device</button>"
                "<button onclick='factoryReset()' style='background-color: #ff4444;'>Factory Reset</button>"
            "</div>"
            "</div>";

    page += "<script>"
                "const currentId = '" + ConfigManager::getCalendarId() + "';"
                "function restartDevice() {"
                    "if (confirm('Are you sure you want to restart the device?')) {"
                        "fetch('/restart', { method: 'POST' })"
                            ".then(() => alert('Device is restarting...'))"
                            ".catch(err => alert('Error: ' + err));"
                    "}"
                "}"
                "function factoryReset() {"
                    "if (confirm('WARNING: This will erase all settings and return the device to factory defaults.\\n\\nAre you sure you want to continue?')) {"
                        "fetch('/factory-reset', { method: 'POST' })"
                            ".then(() => alert('Device is resetting and will restart...'))"
                            ".catch(err => alert('Error: ' + err));"
                    "}"
                "}"
            "</script>"
            "<script>"
                "fetch('/calendars')"
                    ".then(response => response.json())"
                    ".then(data => {"
                        "const select = document.querySelector('#calendar_id');"
                        "select.innerHTML = '';"
                        "const currentId = '" + ConfigManager::getCalendarId() + "';"

                        "const primaryOption = document.createElement('option');"
                        "primaryOption.value = 'primary';"
                        "primaryOption.text = 'Main Calendar';"
                        "primaryOption.selected = currentId === 'primary';"
                        "select.appendChild(primaryOption);"

                        "data.items.forEach(cal => {"
                            "if (cal.id !== 'primary') {"
                                "const option = document.createElement('option');"
                                "option.value = cal.id;"
                                "option.text = cal.summary;"
                                "option.selected = cal.id === currentId;"
                                "select.appendChild(option);"
                            "}"
                        "});"

                        "const selectedCalendar = data.items.find(cal => cal.id === currentId) || "
                            "{summary: currentId === 'primary' ? 'Main Calendar' : 'Unknown Calendar'};"
                        "const statusElement = document.querySelector('#calendar-section .status-cell');"
                        "statusElement.textContent = 'Status: Configured (' + selectedCalendar.summary + ')';"

                        "document.querySelector('#calendar-loading').style.display = 'none';"
                        "select.style.display = 'block';"

                        "if (currentId) {"
                            "updateCalendar(new Event('load'));"
                        "}"
                    "})"
                    ".catch(error => {"
                        "document.querySelector('#calendar-loading').textContent = 'Error loading calendars';"
                        "console.error('Error:', error);"
                    "});"

                "function updateCalendar(event) {"
                    "event.preventDefault();"
                    "const select = document.querySelector('#calendar_id');"
                    "const calendarId = select.value;"
                    "const selectedOption = select.options[select.selectedIndex];"

                    "const statusElement = document.querySelector('#calendar-section .status-cell');"
                    "statusElement.textContent = 'Status: Saving...';"

                    "const upcomingDiv = document.querySelector('#upcoming-bins');"
                    "const upcomingList = document.querySelector('#upcoming-bins-list');"
                    "upcomingList.innerHTML = '<li>Loading new calendar schedule...</li>';"
                    "upcomingDiv.style.display = 'block';"

                    "fetch('/save-calendar', {"
                        "method: 'POST',"
                        "headers: {'Content-Type': 'application/x-www-form-urlencoded'},"
                        "body: 'calendar_id=' + encodeURIComponent(calendarId)"
                    "})"
                    ".then(response => {"
                        "if (!response.ok) throw new Error('Failed to save calendar');"

                        "statusElement.textContent = 'Status: Configured (' + selectedOption.text + ')';"

                        "return fetch('/upcoming-bins');"
                    "})"
                    ".then(response => response.json())"
                    ".then(data => {"
                        "upcomingList.innerHTML = '';"
                        "const events = data.items || [];"
                        "if (events.length === 0) {"
                            "upcomingList.innerHTML = '<li>No collections scheduled</li>';"
                        "} else {"
                            "events.forEach(event => {"
                                "const date = new Date(event.start.date || event.start.dateTime);"
                                "const day = date.toLocaleDateString('en-GB', { weekday: 'long', month: 'long', day: 'numeric' });"
                                "upcomingList.innerHTML += `<li>${day}: ${event.summary}</li>`;"
                            "});"
                        "}"
                    "})"
                    ".catch(error => {"
                        "console.error('Error:', error);"
                        "statusElement.textContent = 'Error saving calendar';"
                        "upcomingList.innerHTML = '<li>Error loading schedule</li>';"
                        "alert('Failed to update calendar settings');"
                    "});"

                    "return false;"
                "}"
            "</script>"
        "</body>"
        "</html>";

    return page.c_str();
}
