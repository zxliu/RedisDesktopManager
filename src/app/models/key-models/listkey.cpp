#include "listkey.h"
#include "modules/redisclient/command.h"
#include "modules/redisclient/commandexecutor.h"

ListKeyModel::ListKeyModel(QSharedPointer<RedisClient::Connection> connection, QString fullPath, int dbIndex, int ttl)
    : ListLikeKeyModel(connection, fullPath, dbIndex, ttl)
{    
    loadRowCount();
}

QString ListKeyModel::getType()
{
    return "list";
}

void ListKeyModel::updateRow(int rowIndex, const QVariantMap &row)
{
    if (!isRowLoaded(rowIndex) || !isRowValid(row))
        throw Exception("Invalid row");

    if (isActualPositionChanged(rowIndex))
        throw Exception("Can't delete row from list, because row already has changed."
                        " Reload values and try again.");

    QByteArray newRow(row["value"].toByteArray());
    setListRow(rowIndex, newRow);
    m_rowsCache.replace(rowIndex,newRow);
}

void ListKeyModel::addRow(const QVariantMap &row)
{
    if (!isRowValid(row))
        throw Exception("Invalid row");

    addListRow(row["value"].toByteArray());
}

void ListKeyModel::loadRows(unsigned long rowStart, unsigned long count, std::function<void ()> callback)
{
    if (isPartialLoadingSupported()) {
        //TBD
    } else {        
        QStringList rows = getRowsRange("LRANGE", rowStart, count).toStringList();        

        foreach (QString row, rows) {
            m_rowsCache.push_back(row.toUtf8());
        }
    }

    callback();
}

void ListKeyModel::removeRow(int i)
{
    if (!isRowLoaded(i))
        return;

    if (isActualPositionChanged(i))
        throw Exception("Can't delete row from list, because row already has changed."
                        " Reload values and try again.");

    // Replace value by system string
    QString customSystemValue("---VALUE_REMOVED_BY_RDM---");        
    setListRow(i, customSystemValue);

    // Remove all system values from list
    deleteListRow(0, customSystemValue);

    m_rowCount--;
    m_rowsCache.removeAt(i);

    if (m_rowCount == 0) {
        m_isKeyRemoved = true;
        emit removed();
    }
}

void ListKeyModel::loadRowCount()
{    
    m_rowCount = getRowCount("LLEN");
}

bool ListKeyModel::isActualPositionChanged(int row)
{
    using namespace RedisClient;

    QByteArray cachedValue = m_rowsCache[row];

    // check position update
    Command getValueByIndex(QStringList()
                            << "LRANGE" << m_keyFullPath
                            << QString::number(row) << QString::number(row),
                            m_dbIndex);

    Response result = CommandExecutor::execute(m_connection, getValueByIndex);

    QStringList currentState = result.getValue().toStringList();

    return currentState.size() != 1 || currentState[0] != QString(cachedValue);
}

void ListKeyModel::addListRow(const QString &value)
{
    using namespace RedisClient;
    Command addCmd(QStringList() << "LPUSH" << m_keyFullPath << value, m_dbIndex);
    CommandExecutor::execute(m_connection, addCmd);
}

void ListKeyModel::setListRow(int pos, const QString &value)
{
    using namespace RedisClient;
    Command addCmd(QStringList() << "LSET"
                   << m_keyFullPath << QString::number(pos)
                   << value, m_dbIndex);
    CommandExecutor::execute(m_connection, addCmd);
}

void ListKeyModel::deleteListRow(int count, const QString &value)
{
    using namespace RedisClient;
    Command deleteCmd(QStringList() << "LREM" << m_keyFullPath
                   << QString::number(count) << value, m_dbIndex);
    CommandExecutor::execute(m_connection, deleteCmd);
}


