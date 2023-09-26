#include <counoscore/notifications.h>

#include <counoscore/log.h>
#include <counoscore/utilscounosh.h>
#include <counoscore/version.h>

#include <validation.h>
#include <util/system.h>
#include <ui_interface.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <stdint.h>
#include <string>
#include <vector>

namespace mastercore
{

//! Vector of currently active Counos alerts
std::vector<AlertData> currentCounosAlerts;

/**
 * Deletes previously broadcast alerts from sender from the alerts vector
 *
 * Note cannot be used to delete alerts from other addresses, nor to delete system generated feature alerts
 */
void DeleteAlerts(const std::string& sender)
{
    for (std::vector<AlertData>::iterator it = currentCounosAlerts.begin(); it != currentCounosAlerts.end(); ) {
        AlertData alert = *it;
        if (sender == alert.alert_sender) {
            PrintToLog("Removing deleted alert (from:%s type:%d expiry:%d message:%s)\n", alert.alert_sender,
                alert.alert_type, alert.alert_expiry, alert.alert_message);
            it = currentCounosAlerts.erase(it);
            uiInterface.CounosStateChanged();
        } else {
            it++;
        }
    }
}

/**
 * Removes all active alerts.
 *
 * A signal is fired to notify the UI about the status update.
 */
void ClearAlerts()
{
    currentCounosAlerts.clear();
    uiInterface.CounosStateChanged();
}

/**
 * Adds a new alert to the alerts vector
 *
 */
void AddAlert(const std::string& sender, uint16_t alertType, uint32_t alertExpiry, const std::string& alertMessage)
{
    AlertData newAlert;
    newAlert.alert_sender = sender;
    newAlert.alert_type = alertType;
    newAlert.alert_expiry = alertExpiry;
    newAlert.alert_message = alertMessage;

    // very basic sanity checks for broadcast alerts to catch malformed packets
    if (sender != "counoscore" && (alertType < ALERT_BLOCK_EXPIRY || alertType > ALERT_CLIENT_VERSION_EXPIRY)) {
        PrintToLog("New alert REJECTED (alert type not recognized): %s, %d, %d, %s\n", sender, alertType, alertExpiry, alertMessage);
        return;
    }

    currentCounosAlerts.push_back(newAlert);
    PrintToLog("New alert added: %s, %d, %d, %s\n", sender, alertType, alertExpiry, alertMessage);
}

/**
 * Determines whether the sender is an authorized source for Counos Core alerts.
 *
 * The option "-counosalertallowsender=source" can be used to whitelist additional sources,
 * and the option "-counosalertignoresender=source" can be used to ignore a source.
 *
 * To consider any alert as authorized, "-counosalertallowsender=any" can be used. This
 * should only be done for testing purposes!
 */
bool CheckAlertAuthorization(const std::string& sender)
{
    std::set<std::string> whitelisted;

    // Mainnet
    whitelisted.insert("cch1qzw6weqkwfdgkmd87js423dmjxycpd6yvaxdqpx"); // Craig   <craig@counos.foundation>
    whitelisted.insert("cch1qnk50rkp0dujuzyq098nt670gmx3q3xd3gya08g"); // Adam    <adam@counos.foundation>
    whitelisted.insert("cch1qp24c9pcec7nv7m98ty8ewkqne002qpamqyxyl0"); // Zathras <zathras@counos.foundation>
    whitelisted.insert("cch1qjrvfu49ryday06yl43xwveqhddekx8yd6w9ccq"); // dexX7   <dexx@bitwatch.co>
    whitelisted.insert("cch1qhlpw2ddryjz8wu0faqdjxvgnchd2zfgs648xxt"); // multisig (signatories are Craig, Adam, Zathras, dexX7, JR)

    // Testnet / Regtest
    // use -counosalertallowsender for testing

    // Add manually whitelisted sources
    if (gArgs.IsArgSet("-counosalertallowsender")) {
        const std::vector<std::string>& sources = gArgs.GetArgs("-counosalertallowsender");

        for (std::vector<std::string>::const_iterator it = sources.begin(); it != sources.end(); ++it) {
            whitelisted.insert(*it);
        }
    }

    // Remove manually ignored sources
    if (gArgs.IsArgSet("-counosalertignoresender")) {
        const std::vector<std::string>& sources = gArgs.GetArgs("-counosalertignoresender");

        for (std::vector<std::string>::const_iterator it = sources.begin(); it != sources.end(); ++it) {
            whitelisted.erase(*it);
        }
    }

    bool fAuthorized = (whitelisted.count(sender) ||
                        whitelisted.count("any"));

    return fAuthorized;
}

/**
 * Alerts including meta data.
 */
std::vector<AlertData> GetCounosCoreAlerts()
{
    return currentCounosAlerts;
}

/**
 * Human readable alert messages.
 */
std::vector<std::string> GetCounosCoreAlertMessages()
{
    std::vector<std::string> vstr;
    for (std::vector<AlertData>::iterator it = currentCounosAlerts.begin(); it != currentCounosAlerts.end(); it++) {
        vstr.push_back((*it).alert_message);
    }
    return vstr;
}

/**
 * Expires any alerts that need expiring.
 */
bool CheckExpiredAlerts(unsigned int curBlock, uint64_t curTime)
{
    for (std::vector<AlertData>::iterator it = currentCounosAlerts.begin(); it != currentCounosAlerts.end(); ) {
        AlertData alert = *it;
        switch (alert.alert_type) {
            case ALERT_BLOCK_EXPIRY:
                if (curBlock >= alert.alert_expiry) {
                    PrintToLog("Expiring alert (from %s: type:%d expiry:%d message:%s)\n", alert.alert_sender,
                        alert.alert_type, alert.alert_expiry, alert.alert_message);
                    it = currentCounosAlerts.erase(it);
                    uiInterface.CounosStateChanged();
                } else {
                    it++;
                }
            break;
            case ALERT_BLOCKTIME_EXPIRY:
                if (curTime > alert.alert_expiry) {
                    PrintToLog("Expiring alert (from %s: type:%d expiry:%d message:%s)\n", alert.alert_sender,
                        alert.alert_type, alert.alert_expiry, alert.alert_message);
                    it = currentCounosAlerts.erase(it);
                    uiInterface.CounosStateChanged();
                } else {
                    it++;
                }
            break;
            case ALERT_CLIENT_VERSION_EXPIRY:
                if (COUNOSCORE_VERSION > alert.alert_expiry) {
                    PrintToLog("Expiring alert (form: %s type:%d expiry:%d message:%s)\n", alert.alert_sender,
                        alert.alert_type, alert.alert_expiry, alert.alert_message);
                    it = currentCounosAlerts.erase(it);
                    uiInterface.CounosStateChanged();
                } else {
                    it++;
                }
            break;
            default: // unrecognized alert type
                    PrintToLog("Removing invalid alert (from:%s type:%d expiry:%d message:%s)\n", alert.alert_sender,
                        alert.alert_type, alert.alert_expiry, alert.alert_message);
                    it = currentCounosAlerts.erase(it);
                    uiInterface.CounosStateChanged();
            break;
        }
    }
    return true;
}

} // namespace mastercore
