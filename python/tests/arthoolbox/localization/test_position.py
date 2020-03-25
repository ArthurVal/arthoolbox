import logging, pytest

import arthoolbox.localization.position as position

module_log = logging.getLogger(__name__)

@pytest.fixture(
    scope = "module",
    params = [
        coord_type(1,1,1) for coord_type in position.Coordinate.Availables
    ],
    ids = lambda x: str(x)
)
def position_coordinate_1(request):
    return request.param


@pytest.fixture(
    scope = "module",
    params = [
        coord_type(2,2,2) for coord_type in position.Coordinate.Availables
    ],
    ids = lambda x: str(x)
)
def position_coordinate_2(request):
    return request.param


@pytest.fixture(
    scope = "module",
    params = [
        None,
        1,
        "Toto",
        (1,2)
    ],
    ids = lambda x: "Frame: {}".format(x)
)
def position_frame_id(request):
    return request.param


def test_construction(position_coordinate_1, position_frame_id):
    log = module_log.getChild("construction")

    log.info(
        "{0:*^50}".format(" Testing position class construction")
    )

    log.debug("Constructor args: {}".format(position_coordinate_1))
    pos = position.Position(position_coordinate_1, position_frame_id)
    log.debug("Position constructed: {}".format(pos))

    assert repr(position_frame_id) in repr(pos) and \
        repr(position_coordinate_1) in repr(pos), \
        "The position repr should contain the frame_id and the coordinate repr"

    assert pos.frame_id == position_frame_id
    assert (pos.type == type(position_coordinate_1))
    assert pos.get() == position_coordinate_1

    good_coordinates_names = frozenset(pos.type._fields)

    bad_coordinates_names = tuple()
    for fields_tuples in [
            coordinate_components._fields
            for coordinate_components in position.Coordinate.Availables - set((pos.type, ))
    ]:
        bad_coordinates_names += fields_tuples
    bad_coordinates_names = frozenset(bad_coordinates_names) - good_coordinates_names

    log.debug(".get() Should contain: {}".format(good_coordinates_names))
    for good_coord_name in good_coordinates_names:
        assert hasattr(pos.get(), good_coord_name)

    log.debug(".get() Shouldn't contain: {}".format(bad_coordinates_names))
    for bad_coord_name in bad_coordinates_names:
        assert not hasattr(pos.get(), bad_coord_name)


def test_set_coordinates(position_coordinate_1, position_coordinate_2):
    log = module_log.getChild("update")

    log.info("{0:*^50}".format(" Testing position update"))


    pos = position.Position(position_coordinate_1, 'Chocolatine')
    log.debug("Positon: {}".format(pos))
    log.debug("Set coordinates to: {}".format(position_coordinate_2))
    pos.set(position_coordinate_2)
    log.debug("New Position: {}".format(pos))

    assert pos.get() != position_coordinate_1



# def test_update():
#     pos_same_frame = position.Position(
#         initial_position[0] + 5,
#         initial_position[1] + 5,
#         initial_position[2] + 5,
#         initial_position[3]
#     )

#     pos_other_frame = position.Position(
#         initial_position[0] + 10,
#         initial_position[1] + 10,
#         initial_position[2] + 10,
#         initial_position[3] + "_titi"
#     )

#     for overwrite in (True, False):

#         log.info(
#             "Testing {} overwrite_frame_id"
#             .format("with" if overwrite else "without")
#         )

#         for new_pos in (pos_same_frame, pos_other_frame):

#             pos = position.Position(*initial_position)
#             log.debug("Initial: {}".format(pos))
#             log.debug("New    : {}".format(new_pos))

#             try:
#                 pos.update(from_position = new_pos, overwrite_frame_id = overwrite)

#             except NotImplementedError as err:
#                 log.warning("Skipping test because: {}".format(err))
#                 continue

#             log.debug("After  : {}".format(pos))

#             assert pos.x != initial_position[0], \
#                 "Position X should have changed after update"
#             assert pos.y != initial_position[1], \
#                 "Position Y should have changed after update"
#             assert pos.z != initial_position[2], \
#                 "Position Z should have changed after update"
#             assert (pos.x, pos.y, pos.z) == (new_pos.x, new_pos.y, new_pos.z), \
#                 "Position should have the same coordinate as new_position"

#             if overwrite:
#                 assert pos.frame_id == new_pos.frame_id, \
#                     ("Position should have the same frame_id as new_position"
#                      " when overwriting")

#             else:
#                 if new_pos.frame_id != initial_position[3]:
#                     assert pos.frame_id != new_pos.frame_id, \
#                         ("Position frame_id should not be changed after"
#                          " updating without overwrite")

#             pos.x +=1
#             pos.y +=1
#             pos.z +=1
#             pos.frame_id += "Chocolatine"
#             assert pos.x != new_pos.x, \
#                 "Update should have made a copy, not reference"
#             assert pos.y != new_pos.y, \
#                 "Update should have made a copy, not reference"
#             assert pos.z != new_pos.z, \
#                 "Update should have made a copy, not reference"
#             assert pos.frame_id != new_pos.frame_id, \
#                 "Update should have made a copy, not reference"




