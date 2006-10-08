#!BPY

""" Registration info for Blender menus: <- these words are ignored
Name: 'ArchiMap UV Projection Unwrapper'
Blender: 240
Group: 'UV'
Tooltip: 'ArchiMap UV Unwrap mesh faces for all select mesh objects'
"""


__author__ = "Campbell Barton"
__url__ = ("blender", "elysiun")
__version__ = "1.1 12/18/05"

__bpydoc__ = """\
This script projection unwraps the selected faces of a mesh.

it operates on all selected mesh objects, and can be used unwrap
selected faces, or all faces.
"""

# -------------------------------------------------------------------------- 
# Archimap UV Projection Unwrapper v1.1 by Campbell Barton (AKA Ideasman) 
# -------------------------------------------------------------------------- 
# ***** BEGIN GPL LICENSE BLOCK ***** 
# 
# This program is free software; you can redistribute it and/or 
# modify it under the terms of the GNU General Public License 
# as published by the Free Software Foundation; either version 2 
# of the License, or (at your option) any later version. 
# 
# This program is distributed in the hope that it will be useful, 
# but WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
# GNU General Public License for more details. 
# 
# You should have received a copy of the GNU General Public License 
# along with this program; if not, write to the Free Software Foundation, 
# Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. 
# 
# ***** END GPL LICENCE BLOCK ***** 
# -------------------------------------------------------------------------- 


from Blender import Object, Scene, Draw, Window, sys, Mesh, Geometry
from Blender.Mathutils import CrossVecs, Matrix, Vector, RotationMatrix, DotVecs, TriangleArea

from math import cos

DEG_TO_RAD = 0.017453292519943295 # pi/180.0
SMALL_NUM = 0.000000001
BIG_NUM = 1e15

global USER_FILL_HOLES
global USER_FILL_HOLES_QUALITY
USER_FILL_HOLES = None
USER_FILL_HOLES_QUALITY = None

import boxpack2d
reload(boxpack2d) # for developing.

dict_matrix = {}

def pointInTri2D(v, v1, v2, v3):
	global dict_matrix
	
	key = v1.x, v1.y, v2.x, v2.y, v3.x, v3.y
	
	# Commented because its slower to do teh bounds check, we should realy cache the bounds info for each face.
	'''
	# BOUNDS CHECK
	xmin= 1000000
	ymin= 1000000
	
	xmax= -1000000
	ymax= -1000000
	
	for i in (0,2,4):
		x= key[i]
		y= key[i+1]
		
		if xmax<x:	xmax= x
		if ymax<y:	ymax= y
		if xmin>x:	xmin= x
		if ymin>y:	ymin= y	
	
	x= v.x
	y= v.y
	
	if x<xmin or x>xmax or y < ymin or y > ymax:
		return False
	# Done with bounds check
	'''
	try:
		mtx = dict_matrix[key]
		if not mtx:
			return False
	except:
		side1 = v2 - v1
		side2 = v3 - v1
		
		nor = CrossVecs(side1, side2)
		
		l1 = [side1[0], side1[1], side1[2]]
		l2 = [side2[0], side2[1], side2[2]]
		l3 = [nor[0], nor[1], nor[2]]
		
		mtx = Matrix(l1, l2, l3)
		
		# Zero area 2d tri, even tho we throw away zerop area faces
		# the projection UV can result in a zero area UV.
		if not mtx.determinant():
			dict_matrix[key] = None
			return False
		
		mtx.invert()
		
		dict_matrix[key] = mtx
	
	uvw = (v - v1) * mtx
	return 0 <= uvw[0] and 0 <= uvw[1] and uvw[0] + uvw[1] <= 1

	
def boundsIsland(faces):
	minx = maxx = faces[0].uv[0][0] # Set initial bounds.
	miny = maxy = faces[0].uv[0][1]
	# print len(faces), minx, maxx, miny , maxy
	for f in faces:
		for uv in f.uv:
			x= uv.x
			y= uv.y
			if x<minx: minx= x
			if y<miny: miny= y
			if x>maxx: maxx= x
			if y>maxy: maxy= y
	
	return minx, miny, maxx, maxy

"""
def boundsEdgeLoop(edges):
	minx = maxx = edges[0][0] # Set initial bounds.
	miny = maxy = edges[0][1]
	# print len(faces), minx, maxx, miny , maxy
	for ed in edges:
		for pt in ed:
			print 'ass'
			x= pt[0]
			y= pt[1]
			if x<minx: x= minx
			if y<miny: y= miny
			if x>maxx: x= maxx
			if y>maxy: y= maxy
	
	return minx, miny, maxx, maxy
"""

# Turns the islands into a list of unpordered edges (Non internal)
# Onlt for UV's
# only returns outline edges for intersection tests. and unique points.

def island2Edge(island):
	
	# Vert index edges
	edges = {}
	
	unique_points= {}
	
	for f in island:
		f_v= f.v
		f_uv= f.uv
		f_uvkey= map(tuple, f_uv)
		
		
		for vIdx in xrange(len(f_v)):
			
			unique_points[f_uvkey[vIdx]] = f_uv[vIdx]
			
			
			if f_v[vIdx].index > f_v[vIdx-1].index:
				i1= vIdx-1;	i2= vIdx
			else:
				i1= vIdx;	i2= vIdx-1
			
			try:
				edges[ f_uvkey[i1], f_uvkey[i2] ] *= 0 # sets eny edge with more then 1 user to 0 are not returned.
				
			except:
				edges[ f_uvkey[i1], f_uvkey[i2] ] = (f_uv[i1] - f_uv[i2]).length, 
	
	# If 2 are the same then they will be together, but full [a,b] order is not correct.
	
	# Sort by length
	
		
	length_sorted_edges = [(Vector(key[0]), Vector(key[1]), value) for key, value in edges.iteritems() if value != 0]
	length_sorted_edges.sort(lambda A, B: cmp(B[2], A[2]))
		
	# Its okay to leave the length in there.
	#for e in length_sorted_edges:
	#	e.pop(2)
	
	# return edges and unique points
	return length_sorted_edges, [v.__copy__().resize3D() for v in unique_points.itervalues()]
	
# ========================= NOT WORKING????
# Find if a points inside an edge loop, un-orderd.
# pt is and x/y
# edges are a non ordered loop of edges.
# #offsets are the edge x and y offset.
"""
def pointInEdges(pt, edges):
	#
	x1 = pt[0] 
	y1 = pt[1]
	
	# Point to the left of this line.
	x2 = -100000
	y2 = -10000
	intersectCount = 0
	for ed in edges:
		xi, yi = lineIntersection2D(x1,y1, x2,y2, ed[0][0], ed[0][1], ed[1][0], ed[1][1])
		if xi != None: # Is there an intersection.
			intersectCount+=1
	
	return intersectCount % 2
"""

def pointInIsland(pt, island):
	vec1 = Vector(); vec2 = Vector(); vec3 = Vector()	
	for f in island:
		f_uv= f.uv
		vec1.x, vec1.y = f_uv[0]
		vec2.x, vec2.y = f_uv[1]
		vec3.x, vec3.y = f_uv[2]

		if pointInTri2D(pt, vec1, vec2, vec3):
			return True
		
		if len(f) == 4:
			vec1.x, vec1.y = f_uv[0]
			vec2.x, vec2.y = f_uv[2]
			vec3.x, vec3.y = f_uv[3]			
			if pointInTri2D(pt, vec1, vec2, vec3):
				return True
	return False


# box is (left,bottom, right, top)
def islandIntersectUvIsland(source, target, SourceOffset):
	# Is 1 point in the box, inside the vertLoops
	edgeLoopsSource = source[6] # Pretend this is offset
	edgeLoopsTarget = target[6]
	
	# Edge intersect test	
	for ed in edgeLoopsSource:
		for seg in edgeLoopsTarget:
			i = Geometry.LineIntersect2D(\
			seg[0], seg[1], SourceOffset+ed[0], SourceOffset+ed[1])
			if i:
				return 1 # LINE INTERSECTION
	
	# 1 test for source being totally inside target
	SourceOffset.resize3D()
	for pv in source[7]:
		if pointInIsland(pv+SourceOffset, target[0]):
			return 2 # SOURCE INSIDE TARGET
	
	# 2 test for a part of the target being totaly inside the source.
	for pv in target[7]:
		if pointInIsland(pv+SourceOffset, source[0]):
			return 3 # PART OF TARGET INSIDE SOURCE.

	return 0 # NO INTERSECTION




# Returns the X/y Bounds of a list of vectors.
def testNewVecLs2DRotIsBetter(vecs, mat=-1, bestAreaSoFar = -1):
	
	# UV's will never extend this far.
	minx = miny = BIG_NUM
	maxx = maxy = -BIG_NUM
	
	for i, v in enumerate(vecs):
		
		# Do this allong the way
		if mat != -1:
			v = vecs[i] = v*mat
			x= v.x
			y= v.y
			if x<minx: minx= x
			if y<miny: miny= y
			if x>maxx: maxx= x
			if y>maxy: maxy= y
		
		# Spesific to this algo, bail out if we get bigger then the current area
		if bestAreaSoFar != -1 and (maxx-minx) * (maxy-miny) > bestAreaSoFar:
			return (BIG_NUM, None), None
	w = maxx-minx
	h = maxy-miny
	return (w*h, w,h), vecs # Area, vecs
	
# Takes a list of faces that make up a UV island and rotate
# until they optimally fit inside a square.
ROTMAT_2D_POS_90D = RotationMatrix( 90, 2)
ROTMAT_2D_POS_45D = RotationMatrix( 45, 2)

RotMatStepRotation = []
rot_angle = 22.5 #45.0/2
while rot_angle > 0.1:
	RotMatStepRotation.append([\
	 RotationMatrix( rot_angle, 2),\
	 RotationMatrix( -rot_angle, 2)])
	
	rot_angle = rot_angle/2.0
	

def optiRotateUvIsland(faces):
	global currentArea
	
	# Bestfit Rotation
	def best2dRotation(uvVecs, MAT1, MAT2):
		global currentArea
		
		newAreaPos, newfaceProjectionGroupListPos =\
		testNewVecLs2DRotIsBetter(uvVecs[:], MAT1, currentArea[0])
		
		
		# Why do I use newpos here? May as well give the best area to date for an early bailout
		# some slight speed increase in this.
		# If the new rotation is smaller then the existing, we can 
		# avoid copying a list and overwrite the old, crappy one.
		
		if newAreaPos[0] < currentArea[0]:
			newAreaNeg, newfaceProjectionGroupListNeg =\
			testNewVecLs2DRotIsBetter(uvVecs, MAT2, newAreaPos[0])  # Reuse the old bigger list.
		else:
			newAreaNeg, newfaceProjectionGroupListNeg =\
			testNewVecLs2DRotIsBetter(uvVecs[:], MAT2, currentArea[0])  # Cant reuse, make a copy.
		
		
		# Now from the 3 options we need to discover which to use
		# we have cerrentArea/newAreaPos/newAreaNeg
		bestArea = min(currentArea[0], newAreaPos[0], newAreaNeg[0])
		
		if currentArea[0] == bestArea:
			return uvVecs
		elif newAreaPos[0] == bestArea:
			uvVecs = newfaceProjectionGroupListPos
			currentArea = newAreaPos		
		elif newAreaNeg[0] == bestArea:
			uvVecs = newfaceProjectionGroupListNeg
			currentArea = newAreaNeg
		
		return uvVecs
		
	
	# Serialized UV coords to Vectors
	uvVecs = [uv for f in faces  for uv in f.uv]
	
	# Theres a small enough number of these to hard code it
	# rather then a loop.
	
	# Will not modify anything
	currentArea, dummy =\
	testNewVecLs2DRotIsBetter(uvVecs)
	
	
	# Try a 45d rotation
	newAreaPos, newfaceProjectionGroupListPos = testNewVecLs2DRotIsBetter(uvVecs[:], ROTMAT_2D_POS_45D, currentArea[0])
	
	if newAreaPos[0] < currentArea[0]:
		uvVecs = newfaceProjectionGroupListPos
		currentArea = newAreaPos
	# 45d done
	
	# Testcase different rotations and find the onfe that best fits in a square
	for ROTMAT in RotMatStepRotation:
		uvVecs = best2dRotation(uvVecs, ROTMAT[0], ROTMAT[1])
	
	# Only if you want it, make faces verticle!
	if currentArea[1] > currentArea[2]:
		# Rotate 90d
		# Work directly on the list, no need to return a value.
		testNewVecLs2DRotIsBetter(uvVecs, ROTMAT_2D_POS_90D)
	
	
	# Now write the vectors back to the face UV's
	i = 0 # count the serialized uv/vectors
	for f in faces:
		f.uv = [uv for uv in uvVecs[i:len(f)+i] ]
		i += len(f)


# Takes an island list and tries to find concave, hollow areas to pack smaller islands into.
def mergeUvIslands(islandList, islandListArea):
	global USER_FILL_HOLES
	global USER_FILL_HOLES_QUALITY
	
	# Pack islands to bottom LHS
	# Sync with island
	
	#islandTotFaceArea = [] # A list of floats, each island area
	#islandArea = [] # a list of tuples ( area, w,h)
	
	
	decoratedIslandList = []
	
	islandIdx = len(islandList)
	while islandIdx:
		islandIdx-=1
		minx, miny, maxx, maxy = boundsIsland(islandList[islandIdx])
		w, h = maxx-minx, maxy-miny
		
		totFaceArea = 0
		offset= Vector(minx, miny)
		for fIdx, f in enumerate(islandList[islandIdx]):
			for uv in f.uv:
				uv -= offset
			
			totFaceArea += islandListArea[islandIdx][fIdx] # Use Cached area. dont recalculate.
		
		islandBoundsArea = w*h
		efficiency = abs(islandBoundsArea - totFaceArea)
		
		# UV Edge list used for intersections as well as unique points.
		edges, uniqueEdgePoints = island2Edge(islandList[islandIdx])
		
		decoratedIslandList.append([islandList[islandIdx], totFaceArea, efficiency, islandBoundsArea, w,h, edges, uniqueEdgePoints]) 
		
	
	# Sort by island bounding box area, smallest face area first.
	# no.. chance that to most simple edge loop first.
	decoratedIslandListAreaSort =decoratedIslandList[:]
	decoratedIslandListAreaSort.sort(lambda A, B: cmp(A[3], B[3]))
	
	# sort by efficiency, Least Efficient first.
	decoratedIslandListEfficSort = decoratedIslandList[:]
	decoratedIslandListEfficSort.sort(lambda A, B: cmp(B[2], A[2]))
	
	# ================================================== THESE CAN BE TWEAKED.
	# This is a quality value for the number of tests.
	# from 1 to 4, generic quality value is from 1 to 100
	USER_STEP_QUALITY =   ((USER_FILL_HOLES_QUALITY - 1) / 25.0) + 1
	
	# If 100 will test as long as there is enough free space.
	# this is rarely enough, and testing takes a while, so lower quality speeds this up.
	
	# 1 means they have the same quality 
	USER_FREE_SPACE_TO_TEST_QUALITY = 1 + (((100 - USER_FILL_HOLES_QUALITY)/100.0) *5)
	
	#print 'USER_STEP_QUALITY', USER_STEP_QUALITY
	#print 'USER_FREE_SPACE_TO_TEST_QUALITY', USER_FREE_SPACE_TO_TEST_QUALITY
	
	removedCount = 0
	
	areaIslandIdx = 0
	ctrl = Window.Qual.CTRL
	BREAK= False
	while areaIslandIdx < len(decoratedIslandListAreaSort) and not BREAK:
		sourceIsland = decoratedIslandListAreaSort[areaIslandIdx]
		
		# Alredy packed?
		if not sourceIsland[0]:
			areaIslandIdx+=1
		else:
			efficIslandIdx = 0
			while efficIslandIdx < len(decoratedIslandListEfficSort) and not BREAK:
				
				if Window.GetKeyQualifiers() & ctrl:
					BREAK= True
					break
				
				# Now we have 2 islands, is the efficience of the islands lowers theres an
				# increasing likely hood that we can fit merge into the bigger UV island.
				# this ensures a tight fit.
				
				# Just use figures we have about user/unused area to see if they might fit.
				
				targetIsland = decoratedIslandListEfficSort[efficIslandIdx]
				
				
				if sourceIsland[0] == targetIsland[0] or\
				not targetIsland[0] or\
				not sourceIsland[0]:
					pass
				else:
					
					# ([island, totFaceArea, efficiency, islandArea, w,h])
					# Waisted space on target is greater then UV bounding island area.
					
					
					# if targetIsland[3] > (sourceIsland[2]) and\ #
					
					if targetIsland[3] > (sourceIsland[1] * USER_FREE_SPACE_TO_TEST_QUALITY) and\
					targetIsland[4] > sourceIsland[4] and\
					targetIsland[5] > sourceIsland[5]:
						
						# DEBUG # print '%.10f  %.10f' % (targetIsland[3], sourceIsland[1])
						
						# These enough spare space lets move the box until it fits
						
						# How many times does the source fit into the target x/y
						blockTestXUnit = targetIsland[4]/sourceIsland[4]
						blockTestYUnit = targetIsland[5]/sourceIsland[5]
						
						boxLeft = 0
						
						# Distllllance we can move between whilst staying inside the targets bounds.
						testWidth = targetIsland[4] - sourceIsland[4]
						testHeight = targetIsland[5] - sourceIsland[5]
						
						# Increment we move each test. x/y
						xIncrement = (testWidth / (blockTestXUnit * USER_STEP_QUALITY))
						yIncrement = (testHeight / (blockTestYUnit * USER_STEP_QUALITY))
						xIncrement= testWidth/USER_STEP_QUALITY
						yIncrement= testHeight/USER_STEP_QUALITY
						
						# Make sure were not moving less then a 3rg of our width/height
						if xIncrement<sourceIsland[4]/3:
							xIncrement= sourceIsland[4]
						if yIncrement<sourceIsland[5]/3:
							yIncrement= sourceIsland[5]
						
						
						boxLeft = 0 # Start 1 back so we can jump into the loop.
						boxBottom= 0 #-yIncrement
						
						##testcount= 0
						
						while boxBottom <= testHeight:
							# Should we use this? - not needed for now.
							#if Window.GetKeyQualifiers() & ctrl:
							#	BREAK= True
							#	break
							
							##testcount+=1
							#print 'Testing intersect'
							Intersect = islandIntersectUvIsland(sourceIsland, targetIsland, Vector(boxLeft, boxBottom))
							#print 'Done', Intersect
							if Intersect == 1:  # Line intersect, dont bother with this any more
								pass
							
							if Intersect == 2:  # Source inside target
								'''
								We have an intersection, if we are inside the target 
								then move us 1 whole width accross,
								Its possible this is a bad idea since 2 skinny Angular faces
								could join without 1 whole move, but its a lot more optimal to speed this up
								since we have alredy tested for it.
								
								It gives about 10% speedup with minimal errors.
								'''
								#print 'ass'
								# Move the test allong its width + SMALL_NUM
								#boxLeft += sourceIsland[4] + SMALL_NUM
								boxLeft += sourceIsland[4]
							elif Intersect == 0: # No intersection?? Place it.
								# Progress
								removedCount +=1
								Window.DrawProgressBar(0.0, 'Merged: %i islands, Ctrl to finish early.' % removedCount)
								
								# Move faces into new island and offset
								targetIsland[0].extend(sourceIsland[0])
								offset= Vector(boxLeft, boxBottom)
								
								for f in sourceIsland[0]:
									for uv in f.uv:
										uv+= offset
								
								sourceIsland[0][:] = [] # Empty
								

								# Move edge loop into new and offset.
								# targetIsland[6].extend(sourceIsland[6])
								#while sourceIsland[6]:
								targetIsland[6].extend( [ (\
									 (e[0]+offset, e[1]+offset, e[2])\
								) for e in sourceIsland[6] ] )
								
								sourceIsland[6][:] = [] # Empty
								
								# Sort by edge length, reverse so biggest are first.
								targetIsland[6].sort(lambda B,A: cmp(A[2], B[2] ))
								
								targetIsland[7].extend(sourceIsland[7])
								offset= Vector(boxLeft, boxBottom, 0)
								for p in sourceIsland[7]:
									p+= offset
								
								sourceIsland[7][:] = []
								
								
								# Decrement the efficiency
								targetIsland[1]+=sourceIsland[1] # Increment totFaceArea
								targetIsland[2]-=sourceIsland[1] # Decrement efficiency
								# IF we ever used these again, should set to 0, eg
								sourceIsland[2] = 0 # No area is anyone wants to know
								
								break
							
							
							
							# INCREMENR NEXT LOCATION
							if boxLeft > testWidth:
								boxBottom += yIncrement
								boxLeft = 0.0
							else:
								boxLeft += xIncrement
						##print testcount
				
				efficIslandIdx+=1
		areaIslandIdx+=1
	
	# Remove empty islands
	i = len(islandList)
	while i:
		i-=1
		if not islandList[i]:
			del islandList[i] # Can increment islands removed here.
	
	
# Takes groups of faces. assumes face groups are UV groups.
def getUvIslands(faceGroups, faceGroupsArea, me):
	islandList = []
	islandListArea = []
	
	Window.DrawProgressBar(0.0, 'Splitting %d projection groups into UV islands:' % len(faceGroups))
	#print '\tSplitting %d projection groups into UV islands:' % len(faceGroups),
	# Find grouped faces
	
	faceGroupIdx = len(faceGroups)
	
	while faceGroupIdx:
		faceGroupIdx-=1
		faces = faceGroups[faceGroupIdx]
		facesArea = faceGroupsArea[faceGroupIdx]
		# print '.',
		
		faceUsers = [[] for i in xrange(len(me.verts)) ]
		faceUsersArea = [[] for i in xrange(len(me.verts)) ]
		# Do the first face
		fIdx = len(faces)
		for fIdx, f in enumerate(faces):
			for v in f:
				faceUsers[v.index].append(f)
				faceUsersArea[v.index].append(facesArea[fIdx])
				
		
		while 1:			
			
			# This is an index that is used to remember
			# what was the last face that was removed, so we know which faces are new and need to have 
			# faces next to them added into the list
			searchFaceIndex = 0
			
			# Find a face that hasnt been used alredy to start the search with
			newIsland = []
			newIslandArea = []
			while not newIsland:
				hasBeenUsed = 1 # Assume its been used.
				if searchFaceIndex >= len(faces):
					break
				for v in faces[searchFaceIndex]:
					if faces[searchFaceIndex] in faceUsers[v.index]:
						# This has not yet been used, it still being used by a vert
						hasBeenUsed = 0
						break
				if hasBeenUsed == 0:
					newIsland.append(faces.pop(searchFaceIndex))
					newIslandArea.append(facesArea.pop(searchFaceIndex))
				
				searchFaceIndex+=1

			if newIsland == []:
				break
			
			
			# Before we start remove the first, search face from being used.
			for v in newIsland[0]:
				vIdx= v.index
				popoffset = 0
				for fIdx in xrange(len(faceUsers[vIdx])):
					if faceUsers[vIdx][fIdx - popoffset] is newIsland[0]:						
						del faceUsers[vIdx][fIdx - popoffset]
						del faceUsersArea[vIdx][fIdx - popoffset]
						
						popoffset += 1
			
			searchFaceIndex = 0
			while searchFaceIndex != len(newIsland):
				for v in newIsland[searchFaceIndex]:
					vIdx= v.index
					# Loop through all faces that use this vert
					while faceUsers[vIdx]:
						sharedFace = faceUsers[vIdx][-1]
						sharedFaceArea = faceUsersArea[vIdx][-1]
						
						newIsland.append(sharedFace)
						newIslandArea.append(sharedFaceArea)
						# Before we start remove the first, search face from being used.
						for vv in sharedFace:
							#faceUsers = [f for f in faceUsers[vv.index] if f != sharedFace]
							vvIdx= vv.index
							fIdx = 0
							for fIdx in xrange(len(faceUsers[vvIdx])):
								if faceUsers[vvIdx][fIdx] is sharedFace:
									del faceUsers[vvIdx][fIdx]
									del faceUsersArea[vvIdx][fIdx]
									break # Can only be used once.
				
				searchFaceIndex += 1
				
				# If all the faces are done and no face has been added then we can quit
			if newIsland:
				islandList.append(newIsland)
				
				islandListArea.append(newIslandArea)
			
			else:
				print '\t(empty island found, ignoring)'
	
	
	Window.DrawProgressBar(0.1, 'Optimizing Rotation for %i UV Islands' % len(islandList))
	
	for island in islandList:
		optiRotateUvIsland(island)
	
	return islandList, islandListArea
	

def packIslands(islandList, islandListArea):
	if USER_FILL_HOLES:
		Window.DrawProgressBar(0.1, 'Merging Islands (Ctrl: skip merge)...')
		mergeUvIslands(islandList, islandListArea) # Modify in place
		
	
	# Now we have UV islands, we need to pack them.
	
	# Make a synchronised list with the islands
	# so we can box pak the islands.
	boxes2Pack = []
	
	# Keep a list of X/Y offset so we can save time by writing the 
	# uv's and packed data in one pass.
	islandOffsetList = [] 
	
	islandIdx = 0
	
	while islandIdx < len(islandList):
		minx, miny, maxx, maxy = boundsIsland(islandList[islandIdx])
		
		w, h = maxx-minx, maxy-miny
		
		if USER_ISLAND_MARGIN:
			minx -= USER_ISLAND_MARGIN# *w
			miny -= USER_ISLAND_MARGIN# *h
			maxx += USER_ISLAND_MARGIN# *w
			maxy += USER_ISLAND_MARGIN# *h
		
			# recalc width and height
			w, h = maxx-minx, maxy-miny
		
		if w < 0.00001 or h < 0.00001:
			del islandList[islandIdx]
			islandIdx -=1
			continue
		
		'''Save the offset to be applied later,
		we could apply to the UVs now and allign them to the bottom left hand area
		of the UV coords like the box packer imagines they are
		but, its quicker just to remember their offset and
		apply the packing and offset in 1 pass '''
		islandOffsetList.append((minx, miny))
		
		# Add to boxList. use the island idx for the BOX id.
		boxes2Pack.append([islandIdx, w,h])
		islandIdx+=1
		
	# Now we have a list of boxes to pack that syncs
	# with the islands.
	
	#print '\tPacking UV Islands...'
	Window.DrawProgressBar(0.7, 'Packing %i UV Islands...' % len(boxes2Pack) )
	
	time1 = sys.time()
	packWidth, packHeight, packedLs = boxpack2d.boxPackIter(boxes2Pack)
	# print 'Box Packing Time:', sys.time() - time1
	
	#if len(pa	ckedLs) != len(islandList):
	#	raise "Error packed boxes differes from original length"
	
	#print '\tWriting Packed Data to faces'
	Window.DrawProgressBar(0.8, 'Writing Packed Data to faces')
	packedLs.sort(lambda A, B: cmp(A[0] , B[0])) # Sort by ID, so there in sync again
	
	islandIdx = len(islandList)
	# Having these here avoids devide by 0
	if islandIdx:
		
		if USER_STRETCH_ASPECT:
			# Maximize to uv area?? Will write a normalize function.
			xfactor = 1.0 / packWidth
			yfactor = 1.0 / packHeight	
		else:
			# Keep proportions.
			xfactor = yfactor = 1.0 / max(packWidth, packHeight)
	
	while islandIdx:
		islandIdx -=1
		# Write the packed values to the UV's
		
		
		xoffset = packedLs[islandIdx][1] - islandOffsetList[islandIdx][0]
		yoffset = packedLs[islandIdx][2] - islandOffsetList[islandIdx][1]
		
		for f in islandList[islandIdx]: # Offsetting the UV's so they fit in there packed box
			for uv in f.uv:
				uv.x= (uv.x+xoffset) * xfactor
				uv.y= (uv.y+yoffset) * yfactor
			
			

def VectoMat(vec):
	a3 = vec.__copy__().normalize()
	
	up = Vector(0,0,1)
	if abs(DotVecs(a3, up)) == 1.0:
		up = Vector(0,1,0)
	
	a1 = CrossVecs(a3, up).normalize()
	a2 = CrossVecs(a3, a1)
	return Matrix([a1[0], a1[1], a1[2]], [a2[0], a2[1], a2[2]], [a3[0], a3[1], a3[2]])


global ob
ob = None
def main():
	global USER_FILL_HOLES
	global USER_FILL_HOLES_QUALITY
	global USER_STRETCH_ASPECT
	global USER_ISLAND_MARGIN
	
	objects= Scene.GetCurrent().objects
	
	# Use datanames as kesy so as not to unwrap a mesh more then once.
	obList =  dict([(ob.getData(name_only=1), ob) for ob in objects.context if ob.type == 'Mesh'])
	
	
	# Face select object may not be selected.
	ob = objects.active
	if ob and ob.sel == 0 and ob.type == 'Mesh':
		# Add to the list
		obList[ob.getData(name_only=1)] = ob
	del objects
	
	obList = obList.values() # turn from a dict to a list.
	
	if not obList:
		Draw.PupMenu('error, no selected mesh objects')
		return
	
	# Create the variables.
	USER_PROJECTION_LIMIT = Draw.Create(66)
	USER_ONLY_SELECTED_FACES = Draw.Create(1)
	USER_SHARE_SPACE = Draw.Create(1) # Only for hole filling.
	USER_STRETCH_ASPECT = Draw.Create(1) # Only for hole filling.
	USER_ISLAND_MARGIN = Draw.Create(0.0) # Only for hole filling.
	USER_FILL_HOLES = Draw.Create(0)
	USER_FILL_HOLES_QUALITY = Draw.Create(50) # Only for hole filling.
	
	
	pup_block = [\
	'Projection',\
	('Angle Limit:', USER_PROJECTION_LIMIT, 1, 89, 'lower for more projection groups, higher for less distortion.'),\
	('Selected Faces Only', USER_ONLY_SELECTED_FACES, 'Use only selected faces from all selected meshes.'),\
	'UV Layout',\
	('Share Tex Space', USER_SHARE_SPACE, 'Objects Share texture space, map all objects into 1 uvmap.'),\
	('Stretch to bounds', USER_STRETCH_ASPECT, 'Stretch the final output to texture bounds.'),\
	('Island Margin:', USER_ISLAND_MARGIN, 0.0, 0.25, 'Margin to reduce bleed from adjacent islands.'),\
	'Fill in empty areas',\
	('Fill Holes', USER_FILL_HOLES, 'Fill in empty areas reduced texture waistage (slow).'),\
	('Fill Quality:', USER_FILL_HOLES_QUALITY, 1, 100, 'Depends on fill holes, how tightly to fill UV holes, (higher is slower)'),\
	]
	
	# Reuse variable
	if len(obList) == 1:
		ob = "Unwrap %i Selected Mesh"
	else:
		ob = "Unwrap %i Selected Meshes"
	
	# HACK, loop until mouse is lifted.
	'''
	while Window.GetMouseButtons() != 0:
		sys.sleep(10)
	'''
	
	if not Draw.PupBlock(ob % len(obList), pup_block):
		return
	del ob
	
	# Convert from being button types
	USER_PROJECTION_LIMIT = USER_PROJECTION_LIMIT.val
	USER_ONLY_SELECTED_FACES = USER_ONLY_SELECTED_FACES.val
	USER_SHARE_SPACE = USER_SHARE_SPACE.val
	USER_STRETCH_ASPECT = USER_STRETCH_ASPECT.val
	USER_ISLAND_MARGIN = USER_ISLAND_MARGIN.val * 50
	USER_FILL_HOLES = USER_FILL_HOLES.val
	USER_FILL_HOLES_QUALITY = USER_FILL_HOLES_QUALITY.val
	
	
	USER_PROJECTION_LIMIT_CONVERTED = cos(USER_PROJECTION_LIMIT * DEG_TO_RAD)
	USER_PROJECTION_LIMIT_HALF_CONVERTED = cos((USER_PROJECTION_LIMIT/2) * DEG_TO_RAD)
	
	
	# Toggle Edit mode
	is_editmode = Window.EditMode()
	if is_editmode:
		Window.EditMode(0)
	# Assume face select mode! an annoying hack to toggle face select mode because Mesh dosent like faceSelectMode.
	
	if USER_SHARE_SPACE:
		# Sort by data name so we get consistand results
		obList.sort(lambda ob1, ob2: cmp( ob1.getData(name_only=1), ob2.getData(name_only=1) ))
		
		collected_islandList= []
		collected_islandListArea= []
	
	
	Window.WaitCursor(1)
	SELECT_FLAG = Mesh.FaceFlags['SELECT']
	time1 = sys.time()
	for ob in obList:
		me = ob.getData(mesh=1)
		
		if not me.faceUV: # Mesh has no UV Coords, dont bother.
			me.faceUV= True
		
		if USER_ONLY_SELECTED_FACES:
			meshFaces = [f for f in me.faces if f.flag & SELECT_FLAG]
		else:
			meshFaces = me.faces
		
		if not meshFaces:
			continue
		
		#print '\n\n\nArchimap UV Unwrapper, mapping "%s", %i faces.' % (me.name, len(meshFaces))
		Window.DrawProgressBar(0.1, 'Archimap UV Unwrapper, mapping "%s", %i faces.' % (me.name, len(meshFaces)))
		
		# Generate Projection
		projectVecs = [] # We add to this allong the way
		
		# =======
		# Generate a projection list from face normals, this is ment to be smart :)
		
		# make a list of face props that are in sync with meshFaces		
		# Make a Face List that is sorted by area.
		faceListProps = []		
		
		for f in meshFaces:
			area = f.area
			if area <= SMALL_NUM:
				for uv in f.uv: # Assign Dummy UVs
					uv.zero()
				
				print 'found zero area face, removing.'
				
			else:
				# Store all here
				faceListProps.append( (f, area, f.no) )
		
		del meshFaces
		
		faceListProps.sort( lambda A, B: cmp(B[1] , A[1]) ) # Biggest first.
		# Smallest first is slightly more efficient, but if the user cancels early then its better we work on the larger data.
		
		# Generate Projection Vecs
		# 0d is   1.0
		# 180 IS -0.59846
		
		
		# Initialize projectVecs
		newProjectVec = faceListProps[0][2] 
		newProjectFacePropList = [faceListProps[0]]	# Popping stuffs it up.
		
		# Predent that the most unique angke is ages away to start the loop off
		mostUniqueAngle = -1.0
		
		# This is popped
		tempFaceListProps = faceListProps[:]
		
		while 1:
			# If theres none there then start with the largest face
			
			# Pick the face thats most different to all existing angles :)
			mostUniqueAngle = 1.0 # 1.0 is 0d. no difference.
			mostUniqueIndex = 0 # fake
			
			fIdx = len(tempFaceListProps)
			
			while fIdx:
				fIdx-=1
				angleDifference = -1.0 # 180d difference.
				
				# Get the closest vec angle we are to.
				for p in projectVecs:
					temp_angle_diff= DotVecs(p, tempFaceListProps[fIdx][2])
					
					if angleDifference < temp_angle_diff:
						angleDifference= temp_angle_diff
				
				if angleDifference < mostUniqueAngle:
					# We have a new most different angle
					mostUniqueIndex = fIdx
					mostUniqueAngle = angleDifference
				
			
			if mostUniqueAngle < USER_PROJECTION_LIMIT_CONVERTED:
				#print 'adding', mostUniqueAngle, USER_PROJECTION_LIMIT, len(newProjectFacePropList)
				newProjectVec = tempFaceListProps[mostUniqueIndex][2]
				newProjectFacePropList = [tempFaceListProps.pop(mostUniqueIndex)]				
			else:
				if len(projectVecs) >= 1: # Must have at least 2 projections
					break
			
			
			# Now we have found the most different vector, add all the faces that are close.
			fIdx = len(tempFaceListProps)
			while fIdx:
				fIdx -= 1
				
				# Use half the angle limit so we dont overweight faces towards this
				# normal and hog all the faces.
				if DotVecs(newProjectVec, tempFaceListProps[fIdx][2]) > USER_PROJECTION_LIMIT_HALF_CONVERTED:
					newProjectFacePropList.append(tempFaceListProps.pop(fIdx))
			
			
			# Now weight the vector to all its faces, will give a more direct projection
			# if the face its self was not representive of the normal from surrounding faces.
			averageVec = Vector(0,0,0)
			for fprop in newProjectFacePropList:
				averageVec += (fprop[2] * fprop[1]) # / len(newProjectFacePropList)
			
			if averageVec.x != 0 or averageVec.y != 0 or averageVec.z != 0: # Avoid NAN
				averageVec.normalize()				
				projectVecs.append(averageVec)
			
			# Now we have used it, ignore it.
			newProjectFacePropList = []
			
		# If there are only zero area faces then its possible
		# there are no projectionVecs
		if not len(projectVecs):
			Draw.PupMenu('error, no projection vecs where generated, 0 area faces can cause this.')
			return
		
		faceProjectionGroupList =[[] for i in xrange(len(projectVecs)) ]
		faceProjectionGroupListArea =[[] for i in xrange(len(projectVecs)) ]
		
		# We need the area later, and we alredy have calculated it. so store it here.
		#faceProjectionGroupListArea =[[] for i in xrange(len(projectVecs)) ]
		
		# MAP and Arrange # We know there are 3 or 4 faces here 
		fIdx = len(faceListProps)
		while fIdx:
			fIdx-=1
			fvec = faceListProps[fIdx][2]
			i = len(projectVecs)
			
			# Initialize first
			bestAng = DotVecs(fvec, projectVecs[0])
			bestAngIdx = 0
			
			# Cycle through the remaining, first alredy done
			while i-1:
				i-=1
				
				newAng = DotVecs(fvec, projectVecs[i])
				if newAng > bestAng: # Reverse logic for dotvecs
					bestAng = newAng
					bestAngIdx = i
			
			# Store the area for later use.
			faceProjectionGroupList[bestAngIdx].append(faceListProps[fIdx][0])
			faceProjectionGroupListArea[bestAngIdx].append(faceListProps[fIdx][1])
			
		
		# Cull faceProjectionGroupList,
		
		
		# Now faceProjectionGroupList is full of faces that face match the project Vecs list
		i= len(projectVecs)
		while i:
			i-=1
			
			# Account for projectVecs having no faces.
			if not faceProjectionGroupList[i]:
				continue
					
			# Make a projection matrix from a unit length vector.
			MatProj = VectoMat(projectVecs[i])
			
			# Get the faces UV's from the projected vertex.
			for f in faceProjectionGroupList[i]:
				f.uv = [MatProj * v.co for v in f]
		
		
		if USER_SHARE_SPACE:
			# Should we collect and pack later?
			islandList, islandListArea = getUvIslands(faceProjectionGroupList, faceProjectionGroupListArea, me)
			collected_islandList.extend(islandList)
			collected_islandListArea.extend(islandListArea)
			
		else:
			# Should we pack the islands for this 1 object?
			islandList, islandListArea = getUvIslands(faceProjectionGroupList, faceProjectionGroupListArea, me)
			packIslands(islandList, islandListArea)
			
		
		
		# update the mesh here if we need to.
	
	# We want to pack all in 1 go, so pack now
	if USER_SHARE_SPACE:
		Window.DrawProgressBar(0.9, "Box Packing for all objects...")
		packIslands(collected_islandList, collected_islandListArea)
	
	print "ArchiMap time: %.2f" % (sys.time() - time1)
	Window.DrawProgressBar(0.9, "ArchiMap Done, time: %.2f sec." % (sys.time() - time1))
	
	Window.DrawProgressBar(1.0, "")
	Window.WaitCursor(0)
	Window.RedrawAll()

if __name__ == '__main__':
	main()
