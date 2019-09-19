#!/usr/bin/python
#
# Copyright (c) 2017, NVIDIA CORPORATION.  All rights reserved.
#
# NVIDIA Corporation and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA Corporation is strictly prohibited.
#

"""

Appendix:
-------
    1) if blob-type = update then entry tuple format is:
           <binary_name partition_name version binary_operating_mode spec_info>

       for each partition, the script supports multiple binaries for different device:
           if the binary is not FAB specific, spec_info should be "common"
           if the binary is FAB specific, spec_info should be valid spec info for target device

           if the binary is not specfic to operating mode, op_mode should be 0
           if the binary is specfic to operating mode, op_mode should be valid one as per its mode

       example: BUP_generator.py -t update -r rollback.bin -e "cboot.bin EBT 2 0 <spec_info>"

    2) if blob-type = bmp then entry tuple format is:
           <filename bmp-type bmp-resolution>

       currently supported bmp-types: nvidia, lowbattery, charging, charged, fullycharged
       currently supported bmp-resolutions: 480, 720, 810, 1080, 4k, 1200_p. The 'p' of 1200_p
       means the panel is portrait.

       example: BUP_generator.py -t bmp -e "nvidia.bmp nvidia 720"

"""

import sys

if sys.hexversion < 0x02060000:
  print >> sys.stderr, "Python 2.6 or newer is required."
  sys.exit(1)

import os
import sys
import struct
import argparse
import textwrap

top_var = "TOP"

def parse_args():
    parser = argparse.ArgumentParser(description="Package a blob and copies into <OUT>", epilog=__doc__,
                                     formatter_class=argparse.RawTextHelpFormatter)

    parser.add_argument('-t', dest='blob_type', default='update', choices=['update', 'bmp'],
                        help=textwrap.dedent('''\
                             payload package type, default: update
                             for OTA : update
                             for BMP : bmp
                             ''')
                       )
    parser.add_argument('-r', dest='accessory',
                        help=textwrap.dedent('''\
                             header accessory binary
                             for OTA : used to embed rollback config info currently
                             for BMP : not used, hence invalid now
                             ''')
                       )
    parser.add_argument('-e', dest='entry_list', required=True,
                        help=textwrap.dedent('''\
                             binary entry list, format should be like:
                             "<entry tuple 1>; <entry tuple 2>; ... ; <entry tuple N>"
                             ''')
                       )
    params = parser.parse_args()
    return params

class payload():
    gpt_part_name_len_max = 36

    def __init__(self, args):
        self.magic = 'NVIDIA__BLOB__V2'
        self.version = 0x00020000
        self.blob_size_pos = struct.calcsize('=16sI')
        self.header_packing = '=16sIIIIII'
        self.uncomp_size_pos = struct.calcsize('=16sIIIII')
        self.accessory = args.accessory
        self.entry_list = args.entry_list

    def get_binary_name(self, filename):
        if (filename == ""): return filename;
        out_path = os.environ.get("OUT")
        if os.path.isfile(filename):
            binary_name = filename
        else:
            binary_name = os.path.join(out_path, filename)
            if not os.path.isfile(binary_name):
                sys.stderr.write("File %s does not exist\n" % binary_name)
                return
        return binary_name

    def parse_entry_list(self):
        entry_list = self.entry_list.split(';')
        self.entry_info_list = []
        for entry in entry_list:
            entry = entry.strip()
            entry_info = entry.split(' ')
            self.entry_info_list.append(entry_info)

    def fill_header(self, blob):
        self.header_size = struct.calcsize(self.header_packing)

        if self.accessory and os.path.isfile(self.accessory):
            accessory_handle = open(self.accessory, 'rb')
            accessory_handle.seek(0, os.SEEK_END)
            self.header_size += accessory_handle.tell()

        header_tuple = (self.magic, self.version, 0, self.header_size,
                        len(self.entry_info_list), self.blob_type, 0)

        header = struct.pack(self.header_packing, *header_tuple)
        blob.write(header)

        if self.accessory and os.path.isfile(self.accessory):
            accessory_handle.seek(0, os.SEEK_SET)
            blob.write(accessory_handle.read())

    def update_blob_size(self, blob):
        blob.seek(self.blob_size_pos, os.SEEK_SET)
        blobsize = struct.pack('=I', self.blob_size)
        blob.write(blobsize)
        blob.seek(self.uncomp_size_pos, os.SEEK_SET)
        blob.write(blobsize)

    def fill_entry(self, blob):
        empty_entry = struct.pack(self.entry_packing, *self.entry_tuple)
        for i in range(0, len(self.entry_info_list)):
            blob.write(empty_entry)

    def fill_image(self, blob):
        self.entry_update_list = []
        for entry_info in self.entry_info_list:
            binary_handle = open(entry_info[0], 'rb')
            binary_handle.seek(0, os.SEEK_END)
            length = binary_handle.tell()
            offset = blob.tell()

            binary_handle.seek(0, os.SEEK_SET)
            blob.write(binary_handle.read())
            binary_handle.close()

            entry_update = (offset, length)
            self.entry_update_list.append(entry_update)

        self.blob_size = blob.tell()


class update_payload(payload):
    spec_len_max = 64

    def __init__(self, arg):
        payload.__init__(self, arg)
        self.blob_type = 0
        self.entry_packing = '=40sIIII64s'
        self.entry_tuple = ('', 0, 0, 0, 0, '')
        self.param_c = 5
        self.outfile = 'ota.blob'

    def parse_entry_list(self):
        payload.parse_entry_list(self)

        for i in range(0, len(self.entry_info_list)):

            entry_info = self.entry_info_list[i]
            if len(entry_info) != self.param_c:
                print 'Invalid entry tuple:', entry_info
                return

            binary_name = payload.get_binary_name(self, entry_info[0])

            if len(entry_info[1]) > payload.gpt_part_name_len_max:
                sys.stderr.write("ERROR:Partition name too long(>%s) %s\n"\
                    % (payload.gpt_part_name_len_max, entry_info[1]));
                sys.exit(-1);
            part_name = entry_info[1]

            try:
                version = int(entry_info[2])
            except ValueError:
                version = 0

            try:
                op_mode = int(entry_info[3])
            except ValueError:
                op_mode = 0

            if len(entry_info[4]) > self.spec_len_max:
                sys.stderr.write("ERROR:Spec too long(>%s) %s\n"\
                    % (self.spec_len_max, entry_info[4]));
                sys.exit(-1);
            spec_info = entry_info[4]
            if (spec_info == "common"):
                spec_info = ""

            entry_info = (binary_name, part_name, version, op_mode, spec_info)
            self.entry_info_list[i] = entry_info

    def update_entry(self, blob):
        blob.seek(self.header_size, os.SEEK_SET)
        for i in range(0, len(self.entry_info_list)):
            entry_info = self.entry_info_list[i]
            entry_update = self.entry_update_list[i]
            part_name = entry_info[1]
            version = entry_info[2]
            op_mode = entry_info[3]
            spec_info = entry_info[4]
            offset = entry_update[0]
            length = entry_update[1]
            entry_tuple = (part_name, offset, length, version, op_mode, spec_info)
            updated_entry = struct.pack(self.entry_packing, *entry_tuple)
            blob.write(updated_entry)

class bmp_payload(payload):
    tp_table = {'nvidia':0, 'lowbattery':1, 'charging':2, 'charged':3,
                'fullycharged':4, 'sata_fw_ota':5, 'verity_yellow_pause':6,
                'verity_yellow_continue':7, 'verity_orange_pause':8,
                'verity_orange_continue':9, 'verity_red_pause':10,
                'verity_red_continue':11, 'verity_red_stop':12,  }

    res_table = {'480':0, '720':1, '810':2, '1080':3, '4k':4, '1200_p':5, }

    def __init__(self, arg):
        payload.__init__(self, arg)
        self.blob_type = 1
        self.entry_packing = '=IIII36s'
        self.entry_tuple = (0, 0, 0, 0, '')
        self.param_c = 3
        self.outfile = 'bmp.blob'

    def parse_entry_list(self):

        payload.parse_entry_list(self)

        for i in range(0, len(self.entry_info_list)):

            entry_info = self.entry_info_list[i]
            if len(entry_info) != self.param_c:
                print 'Invalid entry tuple:', entry_info
                return

            binary_name = payload.get_binary_name(self, entry_info[0])

            try:
                tp = self.tp_table[entry_info[1]]
            except KeyError:
                tp = 13

            try:
                res = self.res_table[entry_info[2]]
            except KeyError:
                res = 6

            entry_info = (binary_name, tp, res)
            self.entry_info_list[i] = entry_info

    def update_entry(self, blob):
        blob.seek(self.header_size, os.SEEK_SET)
        for i in range(0, len(self.entry_info_list)):
            entry_info = self.entry_info_list[i]
            tp = entry_info[1]
            res = entry_info[2]
            entry_update = self.entry_update_list[i]
            offset = entry_update[0]
            length = entry_update[1]
            entry_tuple = (tp, offset, length, res, '')
            updated_entry = struct.pack(self.entry_packing, *entry_tuple)
            blob.write(updated_entry)


# Main function
def main(arg):

    global top_var

    # Check "TOP" variable is set and is valid
    if not os.environ.has_key("TOP") or not os.path.isdir(os.environ["TOP"]):
        if not os.environ.has_key("ANDROID_BUILD_TOP") or \
                not os.path.isdir(os.environ["ANDROID_BUILD_TOP"]):
            sys.stderr.write("Environment variable TOP not set or invalid.\n")
            return
        else:
            top_var = "ANDROID_BUILD_TOP"

    # Check "OUT" variable is set and is valid
    if not os.environ.has_key("OUT") or not os.path.isdir(os.environ["OUT"]):
        sys.stderr.write("Environment variable OUT not set or invalid.\n")
        return

    print 'PARTITION INFO   :', arg.entry_list

    if arg.blob_type == 'update':
        payload_obj = update_payload(arg)
    elif arg.blob_type == 'bmp':
        payload_obj = bmp_payload(arg)
    else:
        return

    blob = open(payload_obj.outfile, "wb")
    payload_obj.parse_entry_list()
    payload_obj.fill_header(blob)
    payload_obj.fill_entry(blob)
    payload_obj.fill_image(blob)
    payload_obj.update_blob_size(blob)
    payload_obj.update_entry(blob)
    blob.close()

if __name__ == '__main__':
    param = parse_args()
    main(param)
    sys.exit(0)
