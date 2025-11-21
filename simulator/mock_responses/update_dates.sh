#!/bin/bash
# Updates mock calendar event dates to today

TODAY=$(date +%Y-%m-%d)
TOMORROW=$(date -v+1d +%Y-%m-%d 2>/dev/null || date -d "+1 day" +%Y-%m-%d 2>/dev/null)

echo "Updating mock event dates to: $TODAY - $TOMORROW"

# Update recycling events
sed -i.bak "s/\"date\": \"[0-9-]*\"/\"date\": \"$TODAY\"/" calendar/events_recycling.json
sed -i.bak "s/\"date\": \"[0-9-]*\"/\"date\": \"$TODAY\"/" calendar/events_recycling.json

# Update rubbish events  
sed -i.bak "s/\"date\": \"[0-9-]*\"/\"date\": \"$TODAY\"/" calendar/events_rubbish.json
sed -i.bak "s/\"date\": \"[0-9-]*\"/\"date\": \"$TODAY\"/" calendar/events_rubbish.json

# Clean up backup files
rm -f calendar/*.bak

echo "Done! Mock events now use today's date."
