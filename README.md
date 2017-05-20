# C-Stabilizer

** Introduction

This experiment was performed to measure the maximum throughput at eunomia. In https://github.com/chathurilanchana/riak_kv/blob/Riak-Kv-Causal-dev (erlang version of Eunomia service) we couldnt experiment the max throughput due to inefficient data structures in erlang. Therefore, in order to measure max throughput we wrote the eunomia service in c++, modified clients to directly connect to eunomia bypasing the vnodes and measured the throughput.

** Usage

Build With Makefile
Run the Multiplexer server with the necessary parameters

Required Parameters
-----------------------------------------------------------------------------------------

"arg1:Number of vnodes (simulated clients) , arg2:delete log threshold (for gathering stats (how often to print stats)), arg3:labelReceiverPort, arg4:deliveryLabelSize arg5:IgnoreMsgCount (To ignore initial messages (to avoid the influence of startup delays))
