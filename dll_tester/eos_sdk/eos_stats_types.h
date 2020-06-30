// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "eos_common.h"

enum { k_iStatsCallbackBase = 12000 };

#define EOS_Stats_IngestData             EOS_Stats_IngestData001
#define EOS_Stats_IngestStatOptions      EOS_Stats_IngestStatOptions001
#define EOS_Stats_QueryStatsOptions      EOS_Stats_QueryStatsOptions001
#define EOS_Stats_Stat                   EOS_Stats_Stat001
#define EOS_Stats_GetStatCountOptions    EOS_Stats_GetStatCountOptions001
#define EOS_Stats_CopyStatByIndexOptions EOS_Stats_CopyStatByIndexOptions001
#define EOS_Stats_CopyStatByNameOptions  EOS_Stats_CopyStatByNameOptions001

#include <eos_stats_types1.5.0.h>

#define EOS_STATS_INGESTDATA_API_LATEST      EOS_STATS_INGESTDATA_API_001
#define EOS_STATS_INGESTSTAT_API_LATEST      EOS_STATS_INGESTSTAT_API_001
#define EOS_STATS_QUERYSTATS_API_LATEST      EOS_STATS_QUERYSTATS_API_001
#define EOS_STATS_STAT_API_LATEST            EOS_STATS_STAT_API_001
#define EOS_STATS_GETSTATCOUNT_API_LATEST    EOS_STATS_GETSTATCOUNT_API_001
#define EOS_STATS_COPYSTATBYINDEX_API_LATEST EOS_STATS_COPYSTATBYINDEX_API_001
#define EOS_STATS_COPYSTATBYNAME_API_LATEST  EOS_STATS_COPYSTATBYNAME_API_001