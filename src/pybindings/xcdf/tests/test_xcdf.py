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


def test_write(tmp_path):
    # Create the test file
    path = str(tmp_path / "test_file_write.xcdf")

    # Write to file
    with File(path, "w") as f:
        field_A = f.allocate_uint_field("A", 1, "")
        field_B = f.allocate_float_field("B", 0.1, "A")

        # event/row 1
        field_A.add(4)
        for i in np.arange(0, 1, 0.25):
            field_B.add(i)

        f.write()

        # event/row 2
        field_A.add(1)
        field_B.add(6.0)

        f.write()

        # add a comment
        f.add_comment("test_comment")

    # Read back
    events = []
    with File(path, "r") as f:
        file_header = f.comments
        for e in f:
            events.append(e)

    expected_first_event = {"A": 4, "B": np.array([0.0, 0.3, 0.5, 0.8])}
    expected_second_event = {"A": 1, "B": np.array([6.0])}

    assert file_header == ["test_comment"]
    assert len(events) == 2

    for i, field_name in zip(range(2), ["A", "B"]):
        assert sorted(list(events[i].keys())) == sorted(["A", "B"])
        np.testing.assert_allclose(
            events[0][field_name], expected_first_event[field_name]
        )
        np.testing.assert_allclose(
            events[1][field_name], expected_second_event[field_name]
        )


def test_array_to_field(tmp_path):
    # Create the test file
    path = str(tmp_path / "test_array_to_field.xcdf")

    # Write to file
    with File(path, "w") as f:
        field_A = f.allocate_uint_field("A", 1, "")
        field_B = f.allocate_uint_field("B", 1, "A")
        field_C = f.allocate_int_field("C", 1, "A")
        field_D = f.allocate_float_field("D", 0.1, "A")

        field_A.add(5)
        field_B.add(np.arange(5))
        field_C.add(np.arange(-2, 3))
        field_D.add(np.linspace(-1, 1, 5))

        f.write()

    # Read back and check contents
    with File(path, "r") as f:
        for event in f:
            np.testing.assert_array_equal(event["B"], np.arange(5))
            np.testing.assert_array_equal(event["C"], np.arange(-2, 3))
            np.testing.assert_array_equal(event["D"], np.linspace(-1, 1, 5))
