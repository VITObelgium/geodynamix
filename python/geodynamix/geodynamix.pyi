from os import PathLike
from typing import Optional, TypeAlias, Type
import numpy as np
from numpy.typing import DTypeLike

RasterLike: TypeAlias = (
    raster
    # a raster object
    | PathLike
    # the path to a raster file
    | float
    | int
    # a scalar value that will be exapnded to a raster with the same metadata
    | Type[nodata]
    # the nodata value that will be exapnded to a raster with the same metadata
)

class nodata:
    """
    A class representing a nodata value.
    - This class can be used to represent the nodata value for a given raster
    """
    def __init__(self) -> None: ...

class raster_metadata:
    """
    A class containing raster metadata.
    - The spatial extent and projection information
    - The nodata value

    :param rows: the number of rows in the raster
    :param cols: the number of columns in the raster
    :param nodata: the nodata value that is used when that raster is stored to disk
    :param cell_size: the size of a raster cell (can be m or degrees depending on the projection)
    :param xll: the x value of the lower left cell
    :param yll: the y value of the lower left cell
    """
    def __init__(
        self,
        rows: int = 0,
        cols: int = 0,
        nodata: Optional[float] = None,
        cell_size: float = 1.0,
        xll: float = 0.0,
        yll: float = 0.0,
    ) -> None: ...
    @property
    def rows(self) -> int: ...
    @property
    def cols(self) -> int: ...
    @property
    def nodata(self) -> Optional[float]: ...
    @nodata.setter
    def nodata(self, nodata: Optional[float]): ...
    @property
    def xll(self) -> float: ...
    @property
    def yll(self) -> float: ...
    @property
    def cell_size(self) -> float: ...
    @property
    def projected_epsg(self) -> Optional[int]: ...

class raster:
    """
    A class containing a geospatial raster.
    - This a dense raster where all the pixels are stored row by row in contiguous memory
    - The format of the pixel type can be chosen at creation
    - The nodata value is fixed per data Type
        - NAN for floating point types
        - The maximum value of the data type for unsigned types
        - The minimum value of the data type for signed types

    Keyword arguments:
    :param georef: an existing raster_metadata object (when provided rows, cols, nodata, cell_size, xll, yll should be omitted)
    :param dtype: the data type of the raster cells
    :param fill: the initial value of the raster cells (the raster will be filled with nodata if omitted)
    :param rows: the number of rows in the raster
    :param cols: the number of columns in the raster
    :param nodata: the nodata value that is used when that raster is stored to disk
    :param cell_size: the size of a raster cell (can be m or degrees depending on the projection)
    :param xll: the x value of the lower left cell
    :param xll: the y value of the lower left cell
    """
    def __init__(
        self,
        georef: Optional[raster_metadata] = None,
        dtype: DTypeLike = np.float32,
        fill: Optional[int | float] = None,
        rows: int = 0,
        cols: int = 0,
        nodata: Optional[float] = None,
        cell_size: float = 1.0,
        xll: float = 0.0,
        yll: float = 0.0,
    ) -> None: ...
    @property
    def metadata(self) -> raster_metadata: ...
    @property
    def dtype(self) -> np.dtype: ...
    @property
    def array(self) -> np.ndarray: ...

def raster_equal(lhs: RasterLike, rhs: RasterLike) -> bool:
    """
    Compare the two provided rasters for equaliy. Both the metadata and the pixel values are compared.
    The data types of the rasters must be the same.

    Parameters
    ----------
    lhs : RasterLike
    The left hand side raster.
    rhs : RasterLike
    The right hand side raster.
    """
    ...

def read(path: PathLike | str) -> raster: ...
def read_as(dtype: DTypeLike, path: PathLike | str) -> raster: ...
def write(raster: raster, path: PathLike | str) -> raster: ...
