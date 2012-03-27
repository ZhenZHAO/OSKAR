/*
 * Copyright (c) 2011, The University of Oxford
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Oxford nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef OSKAR_ABOUT_H_
#define OSKAR_ABOUT_H_

/**
 * @file oskar_About.h
 */

#include <QtGui/QDialog>

class QDialogButtonBox;
class QLabel;
class QTextEdit;
class QTextDocument;
class QHBoxLayout;
class QVBoxLayout;
class QSpacerItem;

class oskar_About : public QDialog
{
    Q_OBJECT

public:
    oskar_About(QWidget *parent = 0);

private:
    // Widgets.
    QDialogButtonBox* buttons_;
    QLabel* icon_;
    QLabel* title_;
    QLabel* oerc_;
    QLabel* oxford_;
    QLabel* version_;
    QLabel* date_;
    QTextEdit* licence_;
    QTextDocument* licenceText_;
    QLabel* attribution1_;
    QLabel* attribution2_;
    QVBoxLayout* vLayoutMain_;
    QVBoxLayout* vLayout1_;
    QHBoxLayout* hLayout1_;
    QHBoxLayout* hLayout2_;
    QSpacerItem* verticalSpacer_;
};

#endif /* OSKAR_ABOUT_H_ */