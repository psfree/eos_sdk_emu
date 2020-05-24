@echo off

extra\protobuf\win64\bin\protoc.exe -I proto/ --cpp_out proto/ proto/network_proto.proto
pause
