PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE jobs (
  company varchar(63),
  position varchar(63),
  location varchar(127),
  date_applied datetime not null default (datetime('now')),
  status varchar(63),
  progress varchar(63),
  app_link text,
  apply_method varchar(63),
  referrer varchar(63),
  interview_details text,
  latest_update datetime not null default (datetime('now')),
  resume_version varchar(63)
);
COMMIT;
