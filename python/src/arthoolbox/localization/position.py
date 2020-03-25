"""Module use to define Position/Coordinate classes for Position handling

Classes list:
- Coordinate: Simple namespace use to store coordinates and conversions functions
- Position: Handle a postion (ie a coordinate and a frame_id)
"""
import copy, math, collections
from collections import namedtuple

class Coordinate(object):
    """Useless class use as namespace for the coordinates types
    """
    Cartesian = collections.namedtuple(
        'Cartesian',
        ['x', 'y', 'z']
    )

    Cylindrical = collections.namedtuple(
        'Cylindrical',
        ['r', 'theta', 'z']
    )

    Spherical = collections.namedtuple(
        'Spherical',
        ['rho', 'theta', 'phi']
    )

    def __cart_to_sphe(coord):
        _rho = math.sqrt(math.fsum(map(math.pow, coord, (2,2,2))))
        return Coordinate.Spherical(
            rho   = _rho,
            theta = math.acos(coord.z / _rho),
            phi   = math.atan2(coord.y, coord.x),
        )

    Conversion = {
        (Cartesian, Spherical): __cart_to_sphe,
        (Cartesian, Cylindrical):
        lambda coord: Coordinate.Cylindrical(
            r     = math.sqrt(math.pow(coord.x, 2) + math.pow(coord.y, 2)),
            theta = math.atan2(coord.y, coord.x),
            z     = coord.z
        ),

        (Cylindrical, Cartesian):
        lambda coord: Coordinate.Cartesian(
            x = coord.r * math.cos(coord.theta),
            y = coord.r * math.sin(coord.theta),
            z = coord.z
        ),
        (Cylindrical, Spherical):
        lambda coord: Coordinate.Spherical(
            rho   = math.sqrt(math.pow(coord.r, 2) + math.pow(coord.z, 2)),
            theta = math.atan(coord.r / coord.z),
            phi   = coord.theta,
        ),

        (Spherical, Cartesian):
        lambda coord: Coordinate.Cartesian(
            x = coord.rho * math.sin(coord.theta) * math.cos(coord.phi),
            y = coord.rho * math.sin(coord.theta) * math.sin(coord.phi),
            z = coord.rho * math.cos(coord.theta)
        ),
        (Spherical, Cylindrical):
        lambda coord: Coordinate.Cylindrical(
            r     = coord.rho * math.sin(coord.theta),
            theta = coord.phi,
            z     = coord.rho * math.cos(coord.theta),
        ),
    }

    Availables = frozenset(
        (Cartesian, Cylindrical, Spherical)
    )

    @staticmethod
    def convert(coordinate, to_type):
        """Static function to convert from one coordinate system to an other

        Parameters
        ----------
        coordinate: Coordinate.Cartesian() or Coordinate.Polar() or ...
          An instance of one of the coordinate system listed above
        to_type: Coordinate.Cartesian or Coordinate.Polar or ...
          The targeted type to convert in

        Raises
        ------
        AttributeError
          When type(coordinate) or to_type are not contained in Availables
          or when the conversion function doesn't exist
        """
        if type(coordinate) not in Coordinate.Availables:
            raise AttributeError(
                "Unknow coordinate system {}".format(type(coordinate))
            )

        if to_type not in Coordinate.Availables:
            raise AttributeError(
                "Unknow new coordinate system {}".format(to_type)
            )

        conversion_key = (type(coordinate), to_type)
        if conversion_key not in Coordinate.Conversion:
            raise AttributeError(
                "Unknow conversion from {} to {}".format(*conversion_key)
            )

        return Coordinate.Conversion[conversion_key](coordinate)


class Position(object):
    """Class use to represent position, i.e. coordinate + frame

    TODO: Frame_id conversion Tree

    Attributs
    ---------
    frame_id: str
      The frame ID this position is assioted with
    """

    Repr = namedtuple(
        'Position',
        ['frame_id', 'coordinate']
    )

    def __init__(self, initial_coordinates, frame_id = None):
        """Initialize the position

        Parameters
        ----------
        initial_coordinates: Coordinate
        frame_id: str
          Represent the frame this position is associted with
        """
        self.__coordinates = [
            getattr(initial_coordinates, coordinate_name, 0)
            for coordinate_name in initial_coordinates._fields
        ]
        self.__coordinate_type = type(initial_coordinates)


        self.__frame_id = frame_id


    @property
    def frame_id(self):
        return self.__frame_id


    @property
    def type(self):
        return self.__coordinate_type


    def get(self):
        """Return the current Position as namedtuple (immutable)

        Returns
        -------
        namedtuple
          A tuple containing the current position coordinate
        """
        return self.__coordinate_type(
            self.__coordinates[0],
            self.__coordinates[1],
            self.__coordinates[2],
        )


    def set(self, new_coordinate):
        """Set the new position

        Parameters
        ----------
        new_coordinate: Position or Position.Coordinate
          The new position coordinate

        Raises
        ------
        AttributeError
          When the new_coordinate tuple is not convertible to current coords
        """
        if isinstance(new_coordinate, Position):
            self.update_from_position(new_coordinate)

        else:
            self.__coordinates = list(
                new_coordinate if type(new_coordinate) == self.__coordinate_type
                else Coordinate.convert(
                        new_coordinate,
                        to_type = self.__coordinate_type
                )
            )


    def update_from_position(self, position, overwrite_frame_id = False):
        """Update the current object position with respect to new position

        Parameters
        ----------
        from_position: Position
          The new position to update our coordinate with
        overwrite_frame_id: bool
          Indicate if we wish to overwrite our frame_id with from_position
        """

        # NOTE: This is questionnable, does-it have any sense to overwrite a
        # frame_id ? Objects shouldn't change their frame_id
        if overwrite_frame_id:
            self.__frame_id = copy.copy(position.frame_id)

        if position.frame_id == self.frame_id:
            # Simply copy the coordinates
            self.set(position.get())

        else:
            # TODO: Find the from_position coordinate within our frame_id
            raise NotImplementedError(
                ("Currently unable to update a Position from an other"
                 " Position not within the same frame_id")
            )


    def __repr__(self):
         """String representation of a Position
         """
         return str(
             Position.Repr(
                 frame_id = self.frame_id,
                 coordinate = self.get(),
             )
         )
