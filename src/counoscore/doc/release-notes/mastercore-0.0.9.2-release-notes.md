Counos Core v0.0.9.2
==================

v0.0.9.2 is a minor release and not consensus critical in terms of the Counos Layer protocol rules. Due to recent events related to the CounosH network, it is recommended to upgrade to this version.

Please report bugs using the issue tracker on GitHub:

  https://github.com/CounosLayer/counoscore/issues

General
-------

On 4 July 2015, after the activation of [BIP 66](https://github.com/counosh/bips/blob/master/bip-0066.mediawiki), a handful of miners accidentally started to create invalid blocks, which may not be detected by outdated clients. This can result in transaction confirmations that aren't valid on the main chain of the CounosH network.

For further information about this incident, please see:

  https://counosh.org/en/alert/2015-07-04-spv-mining

How to Upgrade
--------------

If you are running an older version, shut it down. Wait until it has completely shut down (which might take a few minutes for older versions), then copy the new version of `mastercored` or `mastercore-qt`.

If you are upgrading from any version earlier than 0.0.9, the first time you run you must start with the `--startclean` parameter at least once to refresh the persistence files.

Downgrading
-----------

Downgrading is currently not supported as older versions will not provide accurate information.

Notable changes
---------------

This release upgrades the underlying code base of Counos Core from CounosH Core 0.9.3 to CounosH Core 0.9.5.

Please see the official release notes of [CounosH Core 0.9.5](release-notes.md), as well as the historical release notes of [Counos Core 0.0.9.1](release-notes/counoscore-0.0.9.1-release-notes.md) for details about recent changes.
