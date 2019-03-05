using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;

namespace controle_de_arquivos_C
{
    public partial class Form1 : Form
    {
        
        public Form1() // iniciar programa
        {
            InitializeComponent();
            label1.Text = "";
            label2.Text = "";
            label3.Text = "";

            Diretorio();
            criar();
        }

        // variaveis
        int pi;
        string nvariaveis;
        string richv;
        string TB1;
        string nt;
        // modificar inicial Label
       



        //adaptado para criar meu diretorio
        private void Diretorio()
        {
            string pasta = "conf"; //nome do diretorio
            if (!Directory.Exists(pasta))
            {
                Directory.CreateDirectory(pasta);
            }
            
        }
        
        private void Deldir()
        {
            string pasta = "conf";
            try
            {
                if (Directory.Exists(pasta) == true)
                {
                    Directory.Delete(pasta);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        
        /// <summary>
        /// http://csharpbrasil.com.br/trabalhando-com-arquivos-em-c-parte-1/
        /// todas explicações no site...
        /// </summary>


        
        private string strPhatfile=@"conf\teste.txt";

       

        //criar arquivo
        private void criar()
        {
            if (!File.Exists(strPhatfile))
            {
                try
                {
                    using (FileStream fs = File.Create(strPhatfile))
                    {
                        using (StreamWriter sw = new StreamWriter(fs))
                        {
                            // se o arquivo não existe crie e carregue o valor das variaveis
                            string nvariaveis = (" novo" + "teste");
                            label3.Text = nvariaveis;
                            richTextBox1.Text = nvariaveis;
                            sw.Write(nvariaveis);
                        }
                    }

                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message);
                }
                MessageBox.Show("Arquivo criado com novas variaveis!!!");
            }
            else
            {
                // se já existe, leia e carregue as variaveis dele aqui
                using (StreamReader sr = new StreamReader(strPhatfile))
                {
                    nvariaveis = sr.ReadToEnd();
                    label3.Text = nvariaveis;
                    richTextBox1.Text = nvariaveis;
                }

            }
            nt = nvariaveis;
            comboBox1.Items.Clear();
            if (nvariaveis != null)
            {
                comboBox1.Items.Add(nvariaveis);
            }
            
               
        }

        private void Abrir()
        {
            try
            {

                if (File.Exists(strPhatfile))
                {
                    System.Diagnostics.Process.Start(strPhatfile);

                }
                else
                {
                    MessageBox.Show("Arquivo não encontrado!");
                }

            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private void substituir()
        {
            try
            {
                if (File.Exists(strPhatfile))
                {
                    File.Delete(strPhatfile);
                }
                else
                {
                    MessageBox.Show("Arquivo não encontrado");
                }
                using (FileStream fs = File.Create(strPhatfile))
                {
                    using (StreamWriter sw = new StreamWriter(fs))
                    {
                        sw.Write(richv);
                    }
                }
                MessageBox.Show("Arquivo atualizado");

            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private void Concatenar()
        {
            try
            {
                if (File.Exists(strPhatfile))
                {
                    using (StreamWriter sw = File.AppendText(strPhatfile))
                    {
                        sw.Write("\r\n"+ TB1);
                    }
                    nt = (nt + "\r\n" + TB1);
                    richTextBox1.Text = nt;
                    label3.Text = (richTextBox1.Text);
                    textBox1.Text = "";
                    textBox1.SelectionLength = textBox1.Text.Length;

                    MessageBox.Show("Arquivo atualizado");
                }
                else
                {
                    MessageBox.Show("Arquivo não encontrado");
                }

            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }


        ///http://csharpbrasil.com.br/trabalhando-com-arquivos-em-c-parte-2/
        ///
        private void Alterar()
        {
            try
            {
                if (File.Exists(strPhatfile))
                {
                    using (FileStream fs = new FileStream(strPhatfile, FileMode.Open, FileAccess.Read))
                    {
                        using (StreamReader sr = new StreamReader(fs))
                        {
                            using(FileStream fsTmp = new FileStream(strPhatfile + ".tmp", FileMode.Create, FileAccess.Write))
                            {
                                using (StreamWriter sw = new StreamWriter(fsTmp))
                                {
                                    string strLinha = null;
                                    while((strLinha = sr.ReadLine()) != null)
                                    {
                                        if (strLinha.IndexOf("adicionado") > -1)
                                        {
                                            strLinha = strLinha.Replace("adicionado", "alterado");
                                        }
                                        sw.Write(strLinha);
                                    }
                                }
                            }
                        }
                    }
                    File.Delete(strPhatfile);
                    File.Move(strPhatfile + ".tmp", strPhatfile);
                    MessageBox.Show("Arquivo alterado com sucesso!");
                }
                else
                {
                    MessageBox.Show("Arquivo não encontrado!");
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }

        }

        //Exclir

        private void Excluir()
        {
            try
            {
                if (File.Exists(strPhatfile))
                {
                    File.Delete(strPhatfile);
                    richTextBox1.Text = "";
                    label3.Text = "";
                    MessageBox.Show("arquivo excluido com sucesso!");
                }
                else
                {
                    MessageBox.Show("Arquivo não encontrado!");
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }




        
        

        private void btCriar_Click(object sender, EventArgs e)
        {
            criar();
            pi += 1;
            string me = Convert.ToString(pi);
            label1.Text = me;  
        }

        private void btAbrir_Click(object sender, EventArgs e)
        {
            Abrir();
        }
        int pi2;

        private void btContatenar_Click(object sender, EventArgs e)
        {
            Concatenar();
            pi2 += 1;
            string me = Convert.ToString(pi2);
            label2.Text = me;
        }

        private void btAlterar_Click(object sender, EventArgs e)
        {
            Alterar();
        }

        private void btExcluir_Click(object sender, EventArgs e)
        {
            Excluir();
        }

        private void btDir_Click(object sender, EventArgs e)
        {
            Diretorio();
        }

        private void btDel_Click(object sender, EventArgs e)
        {
            Deldir();
        }

        private void btSair_Click(object sender, EventArgs e)
        {
            Environment.Exit(1);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            
        }

        private void btRich_Click(object sender, EventArgs e)
        {
            substituir();
        }

        private void richTextBox1_TextChanged(object sender, EventArgs e)
        {
            richv = (richTextBox1.Text);
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            TB1 = (textBox1.Text);
        }
    }
}
