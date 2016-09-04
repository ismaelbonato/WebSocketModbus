/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the QtSerialBus module.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "writeregistermodel.h"

#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QUrl>

enum ModbusConnection {
    Serial,
    Tcp
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , lastRequest(nullptr)
    , modbusDevice(nullptr)
{
    ui->setupUi(this);

    initActions();

    writeModel = new WriteRegisterModel(this);
    writeModel->setStartAddress(ui->writeAddress->value());
    writeModel->setNumberOfValues(ui->writeSize->currentText());

    ui->writeValueTable->setModel(writeModel);
    ui->writeValueTable->hideColumn(2);

    ui->writeValueTable->setColumnHidden(1, 1);
    ui->writeValueTable->setColumnHidden(2, 0);
    ui->writeValueTable->resizeColumnToContents(0);


    connect(writeModel, &WriteRegisterModel::updateViewport, ui->writeValueTable->viewport(),
      static_cast<void (QWidget::*)()>(&QWidget::update));


    ui->writeGroupBox->setEnabled(3);

    on_connectType_currentAdressChanged("127.0.0.1:65000");

    auto model = new QStandardItemModel(10, 1, this);
    for (int i = 0; i < 10; ++i)
        model->setItem(i, new QStandardItem(QStringLiteral("%1").arg(i + 1)));
    ui->writeSize->setModel(model);
    ui->writeSize->setCurrentText("10");
    connect(ui->writeSize,&QComboBox::currentTextChanged, writeModel,
        &WriteRegisterModel::setNumberOfValues);

    auto valueChanged = static_cast<void (QSpinBox::*)(int)> (&QSpinBox::valueChanged);
    connect(ui->writeAddress, valueChanged, writeModel, &WriteRegisterModel::setStartAddress);
    connect(ui->writeAddress, valueChanged, this, [this, model](int i) {
        int lastPossibleIndex = 0;
        const int currentIndex = ui->writeSize->currentIndex();
        for (int ii = 0; ii < 10; ++ii) {
            if (ii < (10 - i)) {
                lastPossibleIndex = ii;
                model->item(ii)->setEnabled(true);
            } else {
                model->item(ii)->setEnabled(false);
            }
        }
        if (currentIndex > lastPossibleIndex)
            ui->writeSize->setCurrentIndex(lastPossibleIndex);
    });
}

MainWindow::~MainWindow()
{
    if (modbusDevice)
        modbusDevice->disconnectDevice();
    delete modbusDevice;

    delete ui;
}

void MainWindow::initActions()
{
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionExit->setEnabled(true);
    ui->actionOptions->setEnabled(true);

}

void MainWindow::on_connectType_currentAdressChanged(const QString Adress)
{
    if (modbusDevice) {
        modbusDevice->disconnectDevice();
        delete modbusDevice;
        modbusDevice = nullptr;
    }

    modbusDevice = new QModbusTcpClient(this);
    if (ui->portEdit->text().isEmpty())
        ui->portEdit->setText(Adress);

    connect(modbusDevice, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
        statusBar()->showMessage(modbusDevice->errorString(), 5000);
    });

    if (!modbusDevice) {
        ui->connectButton->setDisabled(true);
        statusBar()->showMessage(tr("Could not create Modbus client."), 5000);
    } else {
        connect(modbusDevice, &QModbusClient::stateChanged,
                this, &MainWindow::onStateChanged);
    }
}

void MainWindow::on_connectButton_clicked()
{
    if (!modbusDevice)
        return;

    statusBar()->clearMessage();
    if (modbusDevice->state() != QModbusDevice::ConnectedState) {

        const QUrl url = QUrl::fromUserInput(ui->portEdit->text());
        modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, url.port());
        modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, url.host());


        if (!modbusDevice->connectDevice()) {
            statusBar()->showMessage(tr("Connect failed: ") + modbusDevice->errorString(), 5000);
        } else {
            ui->actionConnect->setEnabled(false);
            ui->actionDisconnect->setEnabled(true);
        }
    } else {
        modbusDevice->disconnectDevice();
        ui->actionConnect->setEnabled(true);
        ui->actionDisconnect->setEnabled(false);
    }
}

void MainWindow::onStateChanged(int state)
{
    bool connected = (state != QModbusDevice::UnconnectedState);
    ui->actionConnect->setEnabled(!connected);
    ui->actionDisconnect->setEnabled(connected);

    if (state == QModbusDevice::UnconnectedState)
        ui->connectButton->setText(tr("Connect"));
    else if (state == QModbusDevice::ConnectedState)
        ui->connectButton->setText(tr("Disconnect"));
}

void MainWindow::on_readButton_clicked()
{
    if (!modbusDevice)
        return;
    ui->readValue->clear();
    statusBar()->clearMessage();

    if (auto *reply = modbusDevice->sendReadRequest(readRequest(), ui->serverEdit->value())) {
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &MainWindow::readReady);
        else
            delete reply; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + modbusDevice->errorString(), 5000);
    }
}

void MainWindow::readReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        for (uint i = 0; i < unit.valueCount(); i++) {
            const QString entry = tr("Address: %1, Value: %2").arg(unit.startAddress())
                                     .arg(QString::number(unit.value(i),
                                          unit.registerType() <= QModbusDataUnit::Coils ? 10 : 16));
            ui->readValue->addItem(entry);
        }
    } else if (reply->error() == QModbusDevice::ProtocolError) {
        statusBar()->showMessage(tr("Read response error: %1 (Mobus exception: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->rawResult().exceptionCode(), -1, 16), 5000);
    } else {
        statusBar()->showMessage(tr("Read response error: %1 (code: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->error(), -1, 16), 5000);
    }

    reply->deleteLater();
}

void MainWindow::on_writeButton_clicked()
{
    if (!modbusDevice)
        return;
    statusBar()->clearMessage();

    QModbusDataUnit writeUnit = writeRequest();
    QModbusDataUnit::RegisterType table = writeUnit.registerType();
    for (uint i = 0; i < writeUnit.valueCount(); i++) {
        if (table == QModbusDataUnit::Coils)
            writeUnit.setValue(i, writeModel->m_coils[i + writeUnit.startAddress()]);
        else
            writeUnit.setValue(i, writeModel->m_holdingRegisters[i + writeUnit.startAddress()]);
    }

    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, ui->serverEdit->value())) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                if (reply->error() == QModbusDevice::ProtocolError) {
                    statusBar()->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
                        .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16),
                        5000);
                } else if (reply->error() != QModbusDevice::NoError) {
                    statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
                        arg(reply->errorString()).arg(reply->error(), -1, 16), 5000);
                }
                reply->deleteLater();
            });
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
        statusBar()->showMessage(tr("Write error: ") + modbusDevice->errorString(), 5000);
    }
}

void MainWindow::on_readWriteButton_clicked()
{
    if (!modbusDevice)
        return;
    ui->readValue->clear();
    statusBar()->clearMessage();

    QModbusDataUnit writeUnit = writeRequest();
    QModbusDataUnit::RegisterType table = writeUnit.registerType();
    for (uint i = 0; i < writeUnit.valueCount(); i++) {
        if (table == QModbusDataUnit::Coils)
            writeUnit.setValue(i, writeModel->m_coils[i + writeUnit.startAddress()]);
        else
            writeUnit.setValue(i, writeModel->m_holdingRegisters[i + writeUnit.startAddress()]);
    }

    if (auto *reply = modbusDevice->sendReadWriteRequest(readRequest(), writeUnit,
        ui->serverEdit->value())) {
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &MainWindow::readReady);
        else
            delete reply; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + modbusDevice->errorString(), 5000);
    }
}

QModbusDataUnit MainWindow::readRequest() const
{
    const auto table =
        static_cast<QModbusDataUnit::RegisterType> (QModbusDataUnit::HoldingRegisters);

    int startAddress = ui->readAddress->value();
    Q_ASSERT(startAddress >= 0 && startAddress < 10);

    // do not go beyond 10 entries
    int numberOfEntries = qMin(ui->readSize->currentText().toInt(), 10 - startAddress);
    return QModbusDataUnit(table, startAddress, numberOfEntries);
}

QModbusDataUnit MainWindow::writeRequest() const
{
    const auto table =
        static_cast<QModbusDataUnit::RegisterType> (QModbusDataUnit::HoldingRegisters);

    int startAddress = ui->writeAddress->value();
    Q_ASSERT(startAddress >= 0 && startAddress < 10);

    // do not go beyond 10 entries
    int numberOfEntries = qMin(ui->writeSize->currentText().toInt(), 10 - startAddress);
    return QModbusDataUnit(table, startAddress, numberOfEntries);
}
