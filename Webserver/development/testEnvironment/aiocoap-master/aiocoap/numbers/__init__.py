# This file is part of the Python aiocoap library project.
#
# Copyright (c) 2012-2014 Maciej Wasilak <http://sixpinetrees.blogspot.com/>,
#               2013-2014 Christian Amsüss <c.amsuess@energyharvesting.at>
#
# aiocoap is free software, this file is published under the MIT license as
# described in the accompanying LICENSE file.

"""Module in which all meaningful numbers are collected. Most of the submodules
correspond to IANA registries."""

from . import constants, types, codes
from .constants import *
from .types import *
from .codes import *
from .optionnumbers import OptionNumber

__all__ = constants.__all__ + types.__all__ + codes.__all__ + ['OptionNumber']

media_types = {0: 'text/plain',
               40: 'application/link-format',
               41: 'application/xml',
               42: 'application/octet-stream',
               47: 'application/exi',
               50: 'application/json'}
"""A map from CoAP-assigned integral codes to Internet media type descriptions."""

media_types_rev = {v:k for k, v in media_types.items()}
