'use strict';
/* eslint-disable quotes */

module.exports = [
  {
    "type": "heading",
    "defaultValue": "Theta Configuration"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Color Options"
      },
      {
        "type": "select",
        "label": "Color Mode",
        "messageKey": "colorSetting",
        "defaultValue": "dark",
        "options": 
        [
          {
            "label": "Selected Colors (below)",
            "value": "selectedColors"
          },
          {
            "label": "True Random (may have poor contrast/readability)",
            "value": "trueRandom"
          },
          {
            "label": "Random Dark Colors",
            "value": "dark"
          },
          {
            "label": "Random Light Colors",
            "value": "light"
          },
          {
            "label": "Random Hot Colors",
            "value": "hot"
          },
          {
            "label": "Random Cold Colors",
            "value": "cold"
          }
        ]
      },
      {
        "type": "color",
        "label": "Background Color",
        "messageKey": "backgroundColor",
        "defaultValue": "0000FF",
        "allowGray": true,
        "sunlight": false
      },
      {
        "type": "color",
        "label": "Foreground Color",
        "messageKey": "foregroundColor",
        "defaultValue": "FFFFFF",
        "allowGray": true,
        "sunlight": false
      },
      {
        "type": "text",
        "defaultValue":
          "<font size=3>Random colors change every hour on the hour and when you reload the face</font>"
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Complications"
      },
      {
        "type": "select",
        "label": "Top Line",
        "messageKey": "topLineSetting",
        "defaultValue": "weekdayDate",
        "options": [
          {
            "label": "Digital Time",
            "value": "digitalTime"
          },
          {
            "label": "Month",
            "value": "month"
          },
          {
            "label": "Day of Month",
            "value": "date"
          },
          {
            "label": "Weekday",
            "value": "weekday"
          },
          {
            "label": "Month/Day",
            "value": "monthDay"
          },
          {
            "label": "Day/Month",
            "value": "dayMonth"
          },
          {
            "label": "Weekday, Date",
            "value": "weekdayDate"
          },
          {
            "label": "Steps Today",
            "value": "steps"
          },
          {
            "label": "Distance Walked Today (meters)",
            "value": "meters"
          },
          {
            "label": "Distance Walked Today (feet)",
            "value": "feet"
          },
          {
            "label": "Calories Burned Today",
            "value": "calories"
          }
        ]
      },
      {
        "type": "select",
        "label": "Bottom Line",
        "messageKey": "bottomLineSetting",
        "defaultValue": "steps",
        "options": [
          {
            "label": "Digital Time",
            "value": "digitalTime"
          },
          {
            "label": "Month",
            "value": "month"
          },
          {
            "label": "Day of Month",
            "value": "date"
          },
          {
            "label": "Weekday",
            "value": "weekday"
          },
          {
            "label": "Month/Day",
            "value": "monthDay"
          },
          {
            "label": "Day/Month",
            "value": "dayMonth"
          },
          {
            "label": "Weekday, Date",
            "value": "weekdayDate"
          },
          {
            "label": "Steps Today",
            "value": "steps"
          },
          {
            "label": "Distance Walked Today (meters)",
            "value": "meters"
          },
          {
            "label": "Distance Walked Today (feet)",
            "value": "feet"
          },
          {
            "label": "Calories Burned Today",
            "value": "calories"
          }
        ]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Center Line"
      },
      {
        "type": "select",
        "messageKey": "centerLineSetting",
        "defaultValue": "battery",
        "options": [
          {
            "label": "Center Line Indicates Battery Level",
            "value": "battery"
          },
          {
            "label": "Center Line is Constant Length",
            "value": "constant"
          },
          {
            "label": "No Center Line",
            "value": "none"
          }
        ] 
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Bluetooth"
      },
      {
        "type": "toggle",
        "label": "Vibrate on bluetooth disconnect",
        "messageKey": "bluetoothVibes",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Show icon on bluetooth disconnect",
        "messageKey": "bluetoothIcon",
        "defaultValue": true
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  }
];