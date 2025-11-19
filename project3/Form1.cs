using System;
using System.IO;
using System.Windows.Forms;
using Microsoft.VisualBasic; // For InputBox

namespace project3
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        // Browse folder and load files
        private void btnBrowse_Click(object sender, EventArgs e)
        {
            using (FolderBrowserDialog fbd = new FolderBrowserDialog())
            {
                if (fbd.ShowDialog() == DialogResult.OK)
                {
                    txtFolderPath.Text = fbd.SelectedPath;
                    treeViewFiles.Nodes.Clear();
                    LoadFilesToTree(fbd.SelectedPath);
                }
            }
        }

        // Recursively load folder and txt files into TreeView
        private void LoadFilesToTree(string path, TreeNode parentNode = null)
        {
            DirectoryInfo dir = new DirectoryInfo(path);
            TreeNode node = parentNode ?? treeViewFiles.Nodes.Add(dir.Name);
            node.Tag = dir.FullName; // store full folder path

            // Add subdirectories
            foreach (var subDir in dir.GetDirectories())
            {
                LoadFilesToTree(subDir.FullName, node);
            }

            // Add text files
            foreach (var file in dir.GetFiles("*.txt"))
            {
                TreeNode fileNode = node.Nodes.Add(file.Name);
                fileNode.Tag = file.FullName; // store full file path
            }

            if (parentNode == null) treeViewFiles.ExpandAll();
        }

        // Get full path of selected file (only if it's a file)
        private string GetSelectedFilePath()
        {
            if (treeViewFiles.SelectedNode == null) return null;

            string path = treeViewFiles.SelectedNode.Tag as string;
            return (path != null && File.Exists(path)) ? path : null;
        }

        // Read selected file
        private void btnRead_Click(object sender, EventArgs e)
        {
            string path = GetSelectedFilePath();
            if (path != null)
            {
                txtFileContent.Text = File.ReadAllText(path);
            }
            else
            {
                MessageBox.Show("Please select a valid file.");
            }
        }

        // Update selected file
        private void btnUpdate_Click(object sender, EventArgs e)
        {
            string path = GetSelectedFilePath();
            if (path != null)
            {
                File.WriteAllText(path, txtFileContent.Text);
                MessageBox.Show("File updated!");
            }
            else
            {
                MessageBox.Show("Please select a valid file.");
            }
        }

        // Delete selected file
        private void btnDelete_Click(object sender, EventArgs e)
        {
            string path = GetSelectedFilePath();
            if (path != null)
            {
                var confirm = MessageBox.Show($"Are you sure you want to delete {Path.GetFileName(path)}?",
                    "Confirm Delete", MessageBoxButtons.YesNo);
                if (confirm == DialogResult.Yes)
                {
                    File.Delete(path);
                    treeViewFiles.Nodes.Clear();
                    LoadFilesToTree(txtFolderPath.Text);
                }
            }
            else
            {
                MessageBox.Show("Please select a valid file.");
            }
        }

        // Helper: select TreeNode by full path
        private void SelectNodeByPath(TreeNodeCollection nodes, string fullPath)
        {
            foreach (TreeNode node in nodes)
            {
                if (node.Tag as string == fullPath)
                {
                    treeViewFiles.SelectedNode = node;
                    node.EnsureVisible();
                    return;
                }
                SelectNodeByPath(node.Nodes, fullPath);
            }
        }

        // Create a new file with custom name
        private void btnCreate_Click(object sender, EventArgs e)
        {
            string folder = txtFolderPath.Text;
            if (string.IsNullOrEmpty(folder) || !Directory.Exists(folder))
            {
                MessageBox.Show("Please select a valid folder first.");
                return;
            }

            string newFileName = Interaction.InputBox("Enter new file name (with .txt):", "Create File", "NewFile.txt");
            if (string.IsNullOrWhiteSpace(newFileName)) return;

            string newFilePath = Path.Combine(folder, newFileName);

            if (File.Exists(newFilePath))
            {
                MessageBox.Show("A file with that name already exists.");
                return;
            }

            File.WriteAllText(newFilePath, ""); // create empty file
            treeViewFiles.Nodes.Clear();
            LoadFilesToTree(folder);

            // Select the newly created file
            SelectNodeByPath(treeViewFiles.Nodes, newFilePath);
        }
    }
}
