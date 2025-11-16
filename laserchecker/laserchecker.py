#!/usr/bin/env python3
import requests

# URL to check
url = "https://calendar.boulderlibrary.org/equipment/availability/nextdate?lid=22360&gid=47000&eid=-1&seatId=0&zone=0&capacity=0&isEquipment=false&isSeatBooking=0&pageIndex=0&pageSize=18"

# Your unique ntfy topic (change this to something unique to you)
NTFY_TOPIC = "boulder-laser-availability-max"

def check_laser_availability():
  try:
    response = requests.get(url, timeout=10)
    
    if response.status_code == 200:
      # Send notification via ntfy.sh
      notification_url = f"https://ntfy.sh/{NTFY_TOPIC}"
      requests.post(
        notification_url,
        data="Laser cutting is available at Boulder Library!",
        headers={
          "Title": "Laser Cutter Available",
          "Priority": "high",
          "Tags": "white_check_mark"
        }
      )
      print("Status 200: Notification sent!")
    else:
      print(f"Status {response.status_code}: No notification sent")
      
  except requests.exceptions.RequestException as e:
    print(f"Error checking URL: {e}")

if __name__ == "__main__":
  check_laser_availability()

