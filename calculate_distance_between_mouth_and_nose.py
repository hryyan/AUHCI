import xml.etree.ElementTree as ET
tree = ET.parse("build-AUHCI_crossplatform-Desktop-Debug/CK_database_information.xml")
root = tree.getroot()

class P:
	def __init__(self):
		self.noses_x = 99999
		self.noses_y = 99999
		self.mouths_x = 99999
		self.mouths_y = 99999
		self.offset_x = 0
		self.offset_y = 0
	def valid(self):
		if self.noses_x == 99999 or self.noses_x == 99999 or self.mouths_x == 99999 or self.mouths_y == 99999:
			return False
		else:
			return True
	def getoffset(self):
		self.offset_x = int(self.mouths_x) - int(self.noses_x)
		self.offset_y = int(self.mouths_y) - int(self.noses_y)

l = []

for people in root:
	for expression in people:
		for frame in expression:
			if frame.attrib['Num'] == '1':
				p = P()
				for position in frame:
					if position.tag == "Nose_Position":
						for x in position:
							if x.tag == 'X' and x.text != 99999:
								p.noses_x = int(x.text)
							elif x.tag == 'Y' and x.text != 99999:
								p.noses_y = int(x.text)
					if position.tag == "Mouth_Position":
						for x in position:
							if x.tag == 'X' and x.text != 99999:
								p.mouths_x = int(x.text)
							elif x.tag == 'Y' and x.text != 99999:
								p.mouths_y = int(x.text)
				# print people.attrib['name'], expression.attrib['class'], frame.attrib['jpg'], p.noses_x, p.noses_y, p.mouths_x, p.mouths_y
				if p.valid():
					l.append(p)

offset_x = []
offset_y = []
noses_x = []
noses_y = []
mouths_x = []
mouths_y = []

for p in l:
	# print p.noses_x, p.noses_y, p.mouths_x, p.mouths_y
	p.getoffset()
	offset_x.append(p.offset_x)
	offset_y.append(p.offset_y)
	noses_x.append(p.noses_x)
	noses_y.append(p.noses_y)
	mouths_x.append(p.mouths_x)
	mouths_y.append(p.mouths_y)

print sum(noses_x) / len(noses_x)
print sum(noses_y) / len(noses_y)
print sum(mouths_x) / len(mouths_x)
print sum(mouths_y) / len(mouths_y)
print sum(offset_x) / len(offset_x)
print sum(offset_y) / len(offset_y)
print min(offset_x), max(offset_x)
print min(offset_y), max(offset_y)