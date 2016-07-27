module.exports = [
  {
    "type": "heading",
    "defaultValue": "Watchface Configuration"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Color settings"
      },
      {
        "type": "color",
        "messageKey": "BackgroundColor",
        "defaultValue": "0x000000",
        "label": "Background color"
      },
      {
        "type": "color",
        "messageKey": "ForegroundColor",
        "defaultValue": "0xFFFFFF",
        "label": "Scrolling text color"
      },
	  {
        "type": "color",
        "messageKey": "HourHand",
        "defaultValue": "0xFF0000",
        "label": "Hour hand color"
      },
	  {
        "type": "color",
        "messageKey": "SecondHand",
        "defaultValue": "0xAAAAAA",
        "label": "Second hand color"
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Other settings"
      },
      {
        "type": "toggle",
        "messageKey": "SecondTick",
        "label": "Enable Seconds (off/on)",
        "defaultValue": false
      },
      {
        "type": "toggle",
        "messageKey": "Animations",
        "label": "Shake for scrolling date&time (off/on)",
        "defaultValue": true
      },
	  {
        "type": "toggle",
        "messageKey": "Bluetoothvibe",
        "label": "Bluetooth disconnect vibe (off/on)",
        "defaultValue": false
      },
	  {
        "type": "toggle",
        "messageKey": "Bg",
        "label": "Show dot pattern on analog screen",
        "defaultValue": false
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];