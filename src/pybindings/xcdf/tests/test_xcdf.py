import numpy as np

from xcdf import write_test_file
from xcdf import File


def test_read(tmp_path):

    # Create the test file
    path = str(tmp_path / "testfile.xcdf")
    write_test_file(path)

    # Open the file
    input_file = File(path, "r")

    # Check that 9 Fields have been written
    assert input_file.n_fields == 9

    # Check the only comment
    assert input_file.comments == ["test file"]

    # Check the names of the fields
    assert input_file.field_names == [
        "field1",
        "field2",
        "field6",
        "field8",
        "field3",
        "field4",
        "field5",
        "field7",
        "field9",
    ]

    # Check that there are 1002 events
    assert len(input_file) == 1002

    # Check first event
    actual_first_event = next(input_file)
    expected_first_event = {
        "field1": 2,
        "field2": np.array([1, 1], dtype=np.uint64),
        "field6": 16045690984833335023,
        "field8": np.array([2, 1], dtype=np.uint64),
        "field3": 0.1,
        "field4": 5.0,
        "field5": 5.0,
        "field7": 0.12,
        "field9": np.array([1.0, 2.0, 3.0]),
    }
    np.testing.assert_equal(actual_first_event, expected_first_event)
