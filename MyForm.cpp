#include "MyForm.h"
using namespace System;
using namespace System::Windows::Forms;
using namespace System::Net;
using namespace System::Net::NetworkInformation;
using namespace System::IO;
using namespace System::Collections::Generic;

/// <summary>
/// ������ ���� MyForm
/// </summary>
[STAThread]
void main(array<String^>^ arg) {
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    WatchufEyeProject::MyForm form;
    Application::Run(% form);
}

namespace WatchufEyeProject
{
    void MyForm::button1_Click(System::Object^ sender, System::EventArgs^ e) 
    {
        listBox1->Items->Clear();

        // ���������� ���������� � ������ � sites.txt
        array<String^>^ lines = File::ReadAllLines("sites.txt");

        // ���������� ���������� �� ������� ����� � listBox1
        for each (String ^ line in lines) 
        {
            array<String^>^ parts = line->Split(' ');
            String^ siteName = parts[0];
            String^ siteUrl = parts[1];

            // �������� ������� ��� �������
            HttpWebRequest^ request = dynamic_cast<HttpWebRequest^>(WebRequest::Create(siteUrl));

            // ��������� ������ �� �������
            HttpWebResponse^ response = nullptr;
            try 
            {
                response = dynamic_cast<HttpWebResponse^>(request->GetResponse());
            }
            catch (WebException^ ex) 
            {
                MessageBox::Show("������ ��� ������� � ����� " + siteName + ": " + ex->Message, "������", MessageBoxButtons::OK, MessageBoxIcon::Error);
                continue;
            }

            // ��������� ���������� � �����
            bool sslActivated = (response->SupportsHeaders && response->Headers->Get("Strict-Transport-Security") != nullptr); // ������������ �� SSL ������

            // �������� ����������
            response->Close();

            // ���� �����
            Ping^ pingSender = gcnew Ping();
            Uri^ uri = gcnew Uri(siteUrl);
            String^ host = uri->Host;
            PingReply^ reply = pingSender->Send(host);
            String^ pingInfo = reply->Status == IPStatus::Success ? reply->RoundtripTime.ToString() + " ms" : "����������";

            // ��������� ���������� �������� Uprate
            Random^ rnd = gcnew Random();
            int uprate = rnd->Next(75, 100);

            // ������������ ������ � ����������� � �����
            String^ siteInfo = siteName->PadRight(50) + "\t" +
                siteUrl->PadRight(50) + "\t" +
                (reply->Status == IPStatus::Success ? "��������" : "����������") + "\t\t\t" +
                (sslActivated ? "������������" : "�� ������������") + "\t\t\t" +
                pingInfo->PadRight(50) + "\t" +
                uprate + "%";

            // ���������� ���������� � �����
            listBox1->Items->Add(siteInfo);
        }
    }

    void MyForm::button2_Click(System::Object^ sender, System::EventArgs^ e) 
    {
        if (listBox1->SelectedItem == nullptr) 
        {
            MessageBox::Show("�������� ���� ��� ��������!", "��������������", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }

        // �������� ���������� ��������
        int selectedIndex = listBox1->SelectedIndex;
        listBox1->Items->RemoveAt(selectedIndex);

        array<String^>^ lines = File::ReadAllLines("sites.txt");

        // �������� ������ ������ ����� ��� ��������� ������
        List<String^>^ updatedLines = gcnew List<String^>();
        for (int i = 0; i < lines->Length; i++) 
        {
            if (i != selectedIndex) 
            {
                updatedLines->Add(lines[i]);
            }
        }

        // ���������� sites.txt � ����������� ������� �����
        File::WriteAllLines("sites.txt", updatedLines->ToArray());

        MessageBox::Show("���� ������� ������!", "�����", MessageBoxButtons::OK, MessageBoxIcon::Information);

        // ���������� listBox1 
        button1_Click(sender, e);
    }

    void MyForm::button3_Click(System::Object^ sender, System::EventArgs^ e) {
        String^ siteName = textBox1->Text;
        String^ siteUrl = textBox2->Text;

        // �������� �� ������ �������� � �����
        if (siteName->Trim() == "" || siteUrl->Trim() == "") {
            MessageBox::Show("������� �������� ����� � ������ �� ����!", 
                "��������������", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }

        // �������� �� ������� �������� � ���� ��� �������� �����
        if (siteName->Contains(" ")) {
            MessageBox::Show("�������� ����� �� ������ ��������� �������!", 
                "��������������", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }

        // �������� �� ������������ ������� ������
        if (!siteUrl->StartsWith("http://") && !siteUrl->StartsWith("https://")) {
            MessageBox::Show("������������ ������ ������! ����������, ������� ������ � ������� http://����� ��� https://�����.", 
                "��������������", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }

        // �������� �� ��, ����� ��������� �������� �� �����������
        String^ filePath = "sites.txt";
        if (File::ReadAllText(filePath)->Contains(siteName)) {
            MessageBox::Show("��� ����� ��� ������������, ������� ������ ���!", 
                "��������������", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }

        // ������ � �����
        try 
        {
            // ������� ������ ��� �������
            HttpWebRequest^ request = dynamic_cast<HttpWebRequest^>(WebRequest::Create(siteUrl));

            // �������� ����� �� �������
            HttpWebResponse^ response = dynamic_cast<HttpWebResponse^>(request->GetResponse());

            // ���������, ��� ��� ��������� HTTP ����� 200 (�������� �����)
            if (response->StatusCode != HttpStatusCode::OK) 
            {
                MessageBox::Show("���� ���������� ��� �� ����������", 
                    "������", MessageBoxButtons::OK, MessageBoxIcon::Error);
                response->Close();
                return;
            }

            // �������� ����������
            response->Close();
        }
        catch (Exception^ ex) 
        {
            MessageBox::Show("������ ��� �������� ����������� �����: " + ex->Message, 
                "������", MessageBoxButtons::OK, MessageBoxIcon::Error);
            return;
        }

        // ���������� ����� ������ � sites.txt
        StreamWriter^ sw = File::AppendText(filePath);
        sw->WriteLine(siteName + " " + siteUrl);
        sw->Close();

        textBox1->Text = "";
        textBox2->Text = "";

        MessageBox::Show("���� ������� ��������!", 
            "�����", MessageBoxButtons::OK, MessageBoxIcon::Information);

        // ���������� listBox1
        button1_Click(sender, e);
    }

    void MyForm::button4_Click(System::Object^ sender, System::EventArgs^ e) 
    {
        
        // �������� �� ������ �������� � ����
        if (textBox3->Text->Trim() == "") 
        {
            MessageBox::Show("�� �� ����� ��������!", 
                "��������������", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }

        listBox1->Items->Clear();

        // ���������� ������ �� textBox3
        String^ filter = textBox3->Text->Trim()->ToLower();

        // ���������� ���������� � ������ �� �����
        array<String^>^ lines = File::ReadAllLines("sites.txt");

        // ���������� ���������� �� ������� ����� � listBox1,
        // �������� �������� ������������ ���������� �������� � ������
        for each (String ^ line in lines) 
        {
            array<String^>^ parts = line->Split(' ');
            String^ siteName = parts[0]->ToLower();

            // �������� �� ��, ������������� �� �������� �������� � ������
            if (siteName->StartsWith(filter)) 
            {
                String^ siteUrl = parts[1]; // ������ �� ����

                // �������� ������� ��� �������
                HttpWebRequest^ request = dynamic_cast<HttpWebRequest^>(WebRequest::Create(siteUrl));

                // ��������� ������ �� �������
                HttpWebResponse^ response = nullptr;
                try 
                {
                    response = dynamic_cast<HttpWebResponse^>(request->GetResponse());
                }
                catch (WebException^ ex) 
                {
                    MessageBox::Show("������ ��� ������� � ����� " + siteName + ": " + ex->Message, 
                        "������", MessageBoxButtons::OK, MessageBoxIcon::Error);
                    continue;
                }

                // ��������� ���������� � �����
                bool sslActivated = (response->SupportsHeaders && 
                    response->Headers->Get("Strict-Transport-Security") != nullptr);

                // ��������� ����������
                response->Close();

                // ���� �����
                Ping^ pingSender = gcnew Ping();
                Uri^ uri = gcnew Uri(siteUrl);
                String^ host = uri->Host;
                PingReply^ reply = pingSender->Send(host);
                String^ pingInfo = reply->Status == IPStatus::Success ? reply->RoundtripTime.ToString() 
                    + " ms" : "����������";

                // ��������� ���������� �������� Uprate
                Random^ rnd = gcnew Random();
                int uprate = rnd->Next(50, 100);

                // ������������ ������ � ����������� � �����
                String^ siteInfo = siteName->PadRight(50) + "\t" +
                    siteUrl->PadRight(50) + "\t" +
                    (reply->Status == IPStatus::Success ? "��������" : "����������") + "\t\t\t" +
                    (sslActivated ? "������������" : "�� ������������") + "\t\t\t" +
                    pingInfo->PadRight(50) + "\t" +
                    uprate + "%";

                listBox1->Items->Add(siteInfo);
            }
        }
    }

    void MyForm::button5_Click(System::Object^ sender, System::EventArgs^ e) 
    {
        textBox3->Text = "";

        // ���������� listBox1
        button1_Click(sender, e);      
    }

    void MyForm::textBox1_KeyPress_1(System::Object^ sender, System::Windows::Forms::KeyPressEventArgs^ e) 
    {
        // ������ �� ���� ������������ ��������
        if (e->KeyChar == '.' || e->KeyChar == ',' || e->KeyChar == '\\' || e->KeyChar == '/' ||
            e->KeyChar == '@' || e->KeyChar == '_' || e->KeyChar == '-' || e->KeyChar == '+' ||
            e->KeyChar == '=' || e->KeyChar == '"' || e->KeyChar == '\'' || e->KeyChar == ':' ||
            e->KeyChar == ';' || e->KeyChar == '$' || e->KeyChar == '%' || e->KeyChar == '^' ||
            e->KeyChar == '&' || e->KeyChar == '?' || e->KeyChar == '!' || e->KeyChar == '*' ||
            e->KeyChar == '#' || e->KeyChar == '�' || e->KeyChar == ' ' || e->KeyChar == '`'
            || e->KeyChar == '(' || e->KeyChar == ')' || e->KeyChar == '[' || e->KeyChar == ']'
            || e->KeyChar == '{' || e->KeyChar == '}' || e->KeyChar == '|') {
            e->Handled = true;
        }
        // ���������� ��������� ������� ������� Backspace
        else if (e->KeyChar == '\b') {}

        // ����������� � 5 ��������
        else if (textBox1->Text->Length >= 5) 
        {
            e->Handled = true;
        }
    }
}
