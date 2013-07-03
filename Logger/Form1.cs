using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;
using System.IO;
using System.Threading;

namespace MultiWiiLogger
{
    public partial class Form1 : Form
    {
        static bool StopLogging;
        static SerialPort _serialPort = new SerialPort();
        Thread worker;

        static string data;
        static int freq;

        public Form1()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            StopLogging = false;
            // Create a new SerialPort object with default settings.

            // Allow the user to set the appropriate properties.
            _serialPort.PortName = textBox1.Text;
            _serialPort.BaudRate = 115200;
            //_serialPort.Parity = SetPortParity(_serialPort.Parity);
            _serialPort.DataBits = 8;
            _serialPort.StopBits = StopBits.Two;
            //_serialPort.Handshake = SetPortHandshake(_serialPort.Handshake);

            // Set the read/write timeouts
            _serialPort.ReadTimeout = 5000;
            _serialPort.WriteTimeout = 5000;

            _serialPort.Close();
            _serialPort.Open();

            data = "";

            freq = int.Parse(textBox3.Text);
            worker = new Thread(Read);
            worker.Start();
            

        } 

        private void button2_Click(object sender, EventArgs e)
        {
            StopLogging = true;
            worker.Join(2000);
            textBox2.Text += data;
            string path = @"D:\Desktop\QC_data.csv";
            
            // Create a file to write to. 
            using (StreamWriter sw = File.CreateText(path))
            {
                sw.WriteLine("acc_x;acc_y;acc_z;gyro_roll;gyro_pitch;gyrp_yaw;mag_roll;mag_pitch;mag_yaw;altitude;vario;roll;pitch;yaw;headFreeModeHold;Motor0;Motor1;Motor2;Motor3;Debug0;Debug1;Debug2;Debug3");
                sw.Write(data);
                sw.Flush();
            }
            _serialPort.Close();

            

        }
        
        public void AppendTextBox(string value)
        {
            if (InvokeRequired)
            {
                this.Invoke(new Action<string>(AppendTextBox), new object[] { value });
                return;
            }
            textBox2.Text = value + textBox2.Text;
        }

        public void Read()
        {

            byte[] rec_rc = new byte[24];
            byte[] rec_altitude = new byte[24];
            byte[] rec_attitude = new byte[24];
            byte[] rec_motors = new byte[24];
            byte[] rec_debug = new byte[24];
            byte[] send_buf = new byte[24];
            send_buf[0] = (byte)'$';
            send_buf[1] = (byte)'M';
            send_buf[2] = (byte)'<';
            send_buf[3] = 0;

            byte checksum = 0;
            DateTime t0 = DateTime.Now;

            while (!StopLogging)
            {
                try
                {
                    //RC data
                    send_buf[4] = 102;

                    checksum = 0;
                    checksum ^= send_buf[3];
                    checksum ^= send_buf[4];
                    send_buf[5] = checksum;
                    
                    _serialPort.Write(send_buf, 0, 6);
                    _serialPort.Read(rec_rc, 0, 24);

                    //Attitude data
                    send_buf[4] = 108;

                    checksum = 0;
                    checksum ^= send_buf[3];
                    checksum ^= send_buf[4];
                    send_buf[5] = checksum;


                    _serialPort.Write(send_buf, 0, 6);
                    _serialPort.Read(rec_attitude, 0, 14);


                    //Altitude data
                    send_buf[4] = 109;

                    checksum = 0;
                    checksum ^= send_buf[3];
                    checksum ^= send_buf[4];
                    send_buf[5] = checksum;


                    _serialPort.Write(send_buf, 0, 6);
                    _serialPort.Read(rec_altitude, 0, 12);

                    //Motors
                    send_buf[4] = 104;

                    checksum = 0;
                    checksum ^= send_buf[3];
                    checksum ^= send_buf[4];
                    send_buf[5] = checksum;


                    _serialPort.Write(send_buf, 0, 6);
                    _serialPort.Read(rec_motors, 0, 22);


                    //Debug data
                    send_buf[4] = 254;

                    checksum = 0;
                    checksum ^= send_buf[3];
                    checksum ^= send_buf[4];
                    send_buf[5] = checksum;


                    _serialPort.Write(send_buf, 0, 6);
                    _serialPort.Read(rec_debug, 0, 14);


                    //Set
                    /*
                    send_buf[4] = 211;

                    checksum = 0;
                    checksum ^= send_buf[3];
                    checksum ^= send_buf[4];
                    send_buf[5] = 0;
                    send_buf[6] = 10; //5,6 = Roll
                    send_buf[7] = 0;
                    send_buf[8] = 0; //7,8 = Pitch
                    send_buf[9] = 0;
                    send_buf[10] = 0; //9,10 = Yaw
                    checksum ^= send_buf[5];
                    checksum ^= send_buf[6];
                    checksum ^= send_buf[7];
                    checksum ^= send_buf[8];
                    checksum ^= send_buf[9];
                    checksum ^= send_buf[10];
                    send_buf[11] = checksum;


                    _serialPort.Write(send_buf, 0, 12);
                    */
                    // verwerking
                    string NewData = "";
                    for (int i = 0; i < 9; i++)
                    {
                        NewData += ((short)((short)rec_rc[5 + (i * 2)] + (short)rec_rc[6 + (i * 2)]  << 8)).ToString() + ';';
                    }

                    NewData += ((int)((int)rec_altitude[5]) + ((int)rec_altitude[6] << 8) + ((int)rec_altitude[7] << 16) + ((int)rec_altitude[8] << 24)).ToString() + ';';//altitude
                    NewData += ((short)(((short)rec_altitude[9]) + ((short)rec_altitude[10] << 8))).ToString() + ';';//vario

                    NewData += ((short)(((short)rec_attitude[5]) + ((short)rec_attitude[6] << 8))).ToString() + ';';//angle 1 (roll?)
                    NewData += ((short)(((short)rec_attitude[7]) + ((short)rec_attitude[8] << 8))).ToString() + ';';//angle 2 (pitch?)
                    NewData += ((short)(((short)rec_attitude[9]) + ((short)rec_attitude[10] << 8))).ToString() + ';';//heading(yaw)
                    NewData += ((short)(((short)rec_attitude[11]) + ((short)rec_attitude[12] << 8))).ToString() + ';';//headFreeModeHold?

                    for (int j = 0; j < 4;j++ )
                    {

                        NewData += ((ushort)(((ushort)rec_motors[5 + (2 * j)]) + ((ushort)rec_motors[6 + (2 * j)] << 8))).ToString() + ';';//Motor (1000 vanaf halen)
                    }

                    for (int j = 0; j < 4; j++)
                    {
                        NewData += ((short)(((short)rec_debug[5 + (2 * j)]) + ((short)rec_debug[6 + (2 * j)] << 8))).ToString() + ';';//Debug (signed!)
                    }


                    data += NewData + Environment.NewLine;
                    AppendTextBox(NewData + Environment.NewLine);

                    Thread.Sleep((int)((1.0 / (float)freq) * 1000));
                }
                catch (TimeoutException) { }
            }
        }

        private void textBox2_TextChanged(object sender, EventArgs e)
        {

        }
    }
}
