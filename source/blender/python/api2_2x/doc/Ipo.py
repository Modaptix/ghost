# Blender.Ipo module and the Ipo PyType object

"""
The Blender.Ipo submodule

This module provides access to the Ipo Data in Blender. An Ipo is composed of
several Ipocurves.

A datatype is defined : IpoCurve type. The member functions of this data type
are given below.


Example::
  import Blender
  ob = Blender.Ipo.Get('ipo')    # retrieves an ipo object
  ob.setName('ipo1')
  print ob.name
  print ipo.getRctf()
  ipo.setRctf(1,2,3,4)
  
"""

def New (type, name):
  """
  Creates a new Ipo.
  @type type: string
  @type name: string
  @param type: The Ipo's blocktype. Depends on the object the ipo will be
      linked to. Currently supported types are Object, Camera, World, Material.
  @param name: The name for this Ipo.
  @rtype: Blender Ipo
  @return: The created Ipo.
  """

def Get (name = None):
  """
  Get the Ipo from Blender.
  @type name: string
  @param name: The name of the requested Ipo, or nothing.
  @rtype: Blender Ipo or a list of Blender Ipos
  @return: It depends on the 'name' parameter:
      - (name): The Ipo with the given name;
      - ():     A list with all Ipos in the current scene.
  """

class Ipo:
  """
  The Ipo object
  ==============
  This object gives access to generic data from all objects in Blender.
  It has no attributes.
  """

  def getName():
    """
    Gets the name of the Ipo.
    @rtype: string
    @return: the name of the Ipo.
    """

  def getCurves():
    """
    Gets all the IpoCurves of the Ipo.
    @rtype: list of IpoCurves
    @return: A list (possibly void) containing all the IpoCurves associated to the Ipo object.
    """

  def getCurve(curvename):
    """
    Return the IpoCurve with the given name. The possible values for
    'curvename' are:
      1. Camera Ipo:  Lens, ClSta, ClEnd.
      2. Material Ipo: R, G, B, SpecR, SpecG, SpecB, MirR, MirG, MirB, Ref,
         Alpha, Emit, Amb, Spec, Hard, SpTra, Ang, Mode, HaSize, OfsX, OfsY,
         OfsZ, SizeX, SizeY, SizeZ, TexR, TexG, TexB, DefVar, Col, Nor, Var;
      3. Object Ipo: LocX, LocY, LocZ, dLocX, dLocY, dLocZ, RotX, RotY, RotZ,
         dRotX, dRotY, dRotZ, SizeX, SizeY, SizeZ, dSizeX, dSizeY, dSizeZ,
         Layer, Time, ColR, ColG, ColB, ColA;
      4. World Ipo: HorR, HorG, HorB, ZenR, ZenG, ZenB, Expos, Misi, MisDi,
         MisSta, MisHi, StaR, StaG, StaB, StarDi, StarSi, OfsX, OfsY, OfsZ,
         SizeX, SizeY, SizeZ, TexR, TexG, TexB, DefVar, Col, Nor, Var;
    @type curvename : string
    @rtype: IpoCurve object
    @return: the corresponding IpoCurve, or None.
    """

  def addCurve(curvename):
    """
    Add a new curve to the IPO object. The possible values for 'curvename' are:
      1. Object Ipo: LocX, LocY, LocZ, dLocX, dLocY, dLocZ, RotX, RotY, RotZ,
         dRotX, dRotY, dRotZ, SizeX, SizeY, SizeZ, dSizeX, dSizeY, dSizeZ,
         Layer, Time, ColR, ColG, ColB, ColA.
    @type curvename : string
    @rtype: IpoCurve object
    @return: the corresponding IpoCurve, or None.
    """

  def setName(newname):
    """
    Sets the name of the Ipo.
    @type newname: string
    @rtype: None
    @return: None
    """

  def getBlocktype():
    """
    Gets the blocktype of the Ipo.
    @rtype: int
    @return: the blocktype of the Ipo.
    """

  def setBlocktype(newblocktype):
    """
    Sets the blocktype of the Ipo.
    @type newblocktype: int 
    @rtype: None
    @return: None
    @warn: 'newblocktype' should not be changed unless you really know what
       you are doing ...
    """

  def getRctf():
    """
    Gets the rctf of the Ipo.
    Kind of bounding box...
    @rtype: list of floats
    @return: the rctf of the Ipo.
    """

  def setRctf(newrctf):
    """
    Sets the rctf of the Ipo.
    @type newrctf: four floats.
    @rtype: None
    @return: None
    @warn: rctf should not be changed unless you really know what you are
       doing ...
    """

  def getNcurves():
    """
    Gets the number of curves of the Ipo.
    @rtype: int 
    @return: the number of curve of the Ipo.
    """
    
  def getCurveBP(curvepos):
    """
    Gets the basepoint of a curve of the ipo.
    @type curvepos: int
    @param curvepos: the position of the curve.
    @rtype: a list of 4 floats
    @return: the coordinates of the basepoint, or an error is raised.
    """
    
  def getBeztriple(curvepos,pointpos):
    """
    Gets a beztriple of the Ipo.
    @type curvepos: int
    @param curvepos: the position of the curve in the ipo
    @type pointpos: int
    @param pointpos: the position of the point in the curve.
    @rtype: list of 9 floats
    @return: the beztriple of the Ipo, or an error is raised.
    """

  def setBeztriple(curvepos,pointpos,newbeztriple):
    """
    Sets the beztriple of the Ipo.
    @type curvepos: int
    @param curvepos: the position of the curve in the ipo
    @type pointpos: int
    @param pointpos: the position of the point in the curve.
    @type newbeztriple: list of 9 floats
    @param newbeztriple: the new value for the point
    @rtype: None
    @return: None
    """
    
  def getCurvecurval(curvepos):
    """
    Gets the current value of a curve of the Ipo.
    @type curvepos: int or string
    @param curvepos: the position of the curve in the ipo or the name of the
        curve
    @rtype: float
    @return: the current value of the selected curve of the Ipo.
    """

  def EvaluateCurveOn(curvepos,time):
    """
    Gets the current value of a curve of the Ipo.
    @type curvepos: int
    @param curvepos: the position of the curve in the ipo
    @type time: float
    @param time: the position of the curve in the ipo
    @rtype: float
    @return: the current value of the selected curve of the Ipo at the given
        time.
    """

class IpoCurve:
  """
  The IpoCurve object
  ===================
  This object gives access to generic data from all ipocurves objects in Blender.

  Important Notes for Rotation Curves:\n
  For the rotation IpoCurves, the y values for points are in units of 10 degrees.  example:  45.0 degrees is stored as 4.50 degrees.  These are the same numbers you see in the Transform Properties pupmenu ( NKey ) in the IPO Curve Editor window.  Positive rotations are in a counter-clockwise direction, just like in math class.
  
  @cvar name: The Curve Data name.
  @cvar bezierPoints : The list of the Bezier points.
  """

  def setExtrapolation(extrapolationtype):
    """
    Sets the extrapolation type  of the curve.
    @type extrapolationtype: string
    @param extrapolationtype: the extrapolatrion type of the curve.
        Can be Constant, Extrapolation, Cyclic or Cyclic_extrapolation.
    @rtype: None
    @return: None
    """

  def getExtrapolation():
    """
    Gets the extrapolation type  of the curve.
    @rtype: string
    @return: the extrapolation type  of the curve.Can be Constant, Extrapolation, Cyclic or Cyclic_extrapolation.
    """
    

  def setInterpolation(interpolationtype):
    """
    Sets the interpolation type  of the curve.
    @type interpolationtype: string
    @param interpolationtype: the interpolatrion type of the curve. Can be Constant, Bezier, or Linear.
    @rtype: None
    @return: None
    """
  def getInterpolation():
    """
    Gets the interpolation type  of the curve.
    @rtype: string
    @return: the interpolation type  of the curve.Can be Constant, Bezier, or Linear.
    """
    
  def addBezier(coordlist):
    """
    Adds a Bezier point to a curve.
    @type coordlist: tuple of (at least) 2 floats
    @param coordlist: the x and y coordinates of the new Bezier point.
    @rtype: None
    @return: None
    """

  def Recalc():
    """
    Recomputes the curent value of the curve.
    @rtype: None
    @return: None
    """

  def getName():
    """
    Returns the name of the ipo curve. This name can be:
      1. LocX, LocY, LocZ, dLocX, dLocY, dLocZ, RotX, RotY, RotZ,
         dRotX, dRotY, dRotZ, SizeX, SizeY, SizeZ, dSizeX, dSizeY, dSizeZ,
         Layer, Time, ColR, ColG, ColB, ColA, QuatX, QuatY, QuatZ or QuatW.
    @warn: currently this only works with object and action IPO's..
    @rtype: string
    @return: the name of the ipo curve.
    """

  def getPoints():
    """
    Returns all the points of the ipo curve.
    @rtype: list of BezTriples
    @return: the points of the ipo curve.
    """

  def evaluate( time ):
    """
    Compute the value of the IpoCurve at a particular time.
    @type time: float
    @param time: value along the X axis
    @rtype: float
    @return: the Y value of the curve at the given time
    """


class BezTriple:
  """
  The BezTriple object
  ====================
  This object gives access to generic data from all beztriple objects in Blender.  If an attribute is listed as being 'read-only' that means you cannot write to it.  Use the set*() methods instead.
  @cvar pt : a list of the [x,y] coordinates for knot point of this BezTriple.  read-only.
  @cvar vec : a list of the 3 points [ handle, knot, handle ] that comprise a BezTriple.  See the getTriple() method for an example of the format.  read-only.
  """

  def getPoints():
    """
    Returns the xy coordinates of the Bezier knot point.
    @rtype: list of floats
    @return: list of the x and y coordinates of the Bezier point.
    """

  def setPoints(newval):
    """
    Sets the point xy coordinates of the Bezier knot point.
    @type newval: tuple of 2 floats
    @param newval: the x and y coordinates of the new Bezier point.
    @rtype: None
    @return: None
    """

  def getTriple():
    """
    Returns the x,y,z coordinates for each of the three points that make up
    a BezierTriple.

    The return list looks like this [ [H1x, H1y, H1z], [Px, Py, Pz],
    [H2x, H2y, H2z] ] .

    Example::
      # where bt is of type BezierTriple
      #  and h1, p, and h2 are lists of 3 floats
      h1, p, h2 = bt.getTriple()
    
    @rtype: list consisting of 3 lists of 3 floats
    @return: handle1, knot, handle2
    """
