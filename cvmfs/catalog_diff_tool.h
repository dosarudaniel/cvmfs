/**
 * This file is part of the CernVM File System.
 */

#ifndef CVMFS_CATALOG_DIFF_TOOL_H_
#define CVMFS_CATALOG_DIFF_TOOL_H_

#include <string>

#include "catalog_mgr_ro.h"
#include "statistics.h"
#include "util/pointer.h"

namespace download {
class DownloadManager;
}

class CatalogDiffTool {
 public:
  CatalogDiffTool(const std::string& repo_path, const shash::Any& old_root_hash,
                  const shash::Any& new_root_hash,
                  const std::string& temp_dir_prefix,
                  download::DownloadManager* download_manager);
  virtual ~CatalogDiffTool();

 protected:
  bool Run();
  virtual void ReportAddition(const PathString& path,
                              const catalog::DirectoryEntry& entry,
                              const XattrList& xattrs) = 0;
  virtual void ReportRemoval(const PathString& path,
                             const catalog::DirectoryEntry& entry) = 0;
  virtual void ReportModification(const PathString& path,
                                  const catalog::DirectoryEntry& old_entry,
                                  const catalog::DirectoryEntry& new_entry) = 0;

 private:
  void DiffRec(const PathString& path);

  std::string repo_path_;
  shash::Any old_root_hash_;
  shash::Any new_root_hash_;
  std::string temp_dir_prefix_;

  download::DownloadManager* download_manager_;

  perf::Statistics stats_old_;
  perf::Statistics stats_new_;

  UniquePtr<catalog::SimpleCatalogManager> old_catalog_mgr_;
  UniquePtr<catalog::SimpleCatalogManager> new_catalog_mgr_;
};

#endif  // CVMFS_CATALOG_DIFF_TOOL_H_
