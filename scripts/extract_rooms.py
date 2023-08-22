from genericpath import isdir, isfile
from math import degrees
import xml.etree.ElementTree as ET
import json
import os

class DoorData():
	x = 0
	y = 0
	z = 0
	stage = 0
	room = 0
	cut = 0
	key = 0
	page = 0
	angle = 0

	roomNb = ""

	def __init__(self, door):
		for doorField in door:
			fieldName = doorField.get("name")
			if fieldName == "mPos":
				self.x = float(doorField.get("x"))
				self.y = float(doorField.get("y"))
				self.z = float(doorField.get("z"))
			elif fieldName == "mStage":
				self.stage = int(doorField.get("value"))
			elif fieldName == "mRoom":
				self.room = int(doorField.get("value"))
			elif fieldName == "mCut":
				self.cut = int(doorField.get("value"))
			elif fieldName == "mKey":
				self.key = int(doorField.get("value"))
			elif fieldName == "mPage":
				self.page = int(doorField.get("value"))
			elif fieldName == "mAngY":
				self.angle = degrees(float(doorField.get("value")))
		
		self.roomNb = str(self.stage) + "{:02x}".format(self.room)

	def toJSON(self):
		return {
			"x" : self.x,
			"y" : self.y,
			"z" : self.z,
			"angle" : self.angle,
			"cut" : self.cut,
			"page" : self.page
			}

class RoomData():
	displayName = ""
	doors = []

	def __init__(self, displayName):
		self.displayName = displayName
		self.doors = []

	def toJSON(self):
		return {
			"displayName" : self.displayName,
			"doors" : self.doors
			}

class DataEncoder(json.JSONEncoder):
	def default(self, obj):
		if isinstance(obj, RoomData) or isinstance(obj, DoorData):
			return obj.toJSON()
		return json.JSONEncoder.default(self, obj)

rooms = {}

roomFolders = os.listdir("ext")

for roomNb in roomFolders:
	roomFolder = os.path.join("ext", roomNb)
	if os.path.isdir(roomFolder):
		doorFile = os.path.join(roomFolder, "room", "adr", roomNb + ".adr.xml")
		if os.path.isfile(doorFile):
			tree = ET.parse(doorFile)
			root = tree.getroot()
			for arrays in root.findall("array"):
				for door in arrays.findall("classref"):
					if door.get("type") == "96093989":
							d = DoorData(door)
							if d.roomNb not in rooms:
								rooms[d.roomNb] = RoomData(d.roomNb)
							rooms[d.roomNb].doors.append(d)



with open("rooms.json", "w") as f:
	f.write(json.dumps(rooms, cls=DataEncoder, indent=4))