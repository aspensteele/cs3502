namespace project3
{
    partial class Form1
    {
        private System.ComponentModel.IContainer components = null;
        private System.Windows.Forms.TextBox txtFolderPath;
        private System.Windows.Forms.Button btnBrowse;
        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.TreeView treeViewFiles;
        private System.Windows.Forms.TextBox txtFileContent;
        private System.Windows.Forms.Button btnCreate;
        private System.Windows.Forms.Button btnRead;
        private System.Windows.Forms.Button btnUpdate;
        private System.Windows.Forms.Button btnDelete;

        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        private void InitializeComponent()
        {
            txtFolderPath = new TextBox();
            btnBrowse = new Button();
            splitContainer1 = new SplitContainer();
            treeViewFiles = new TreeView();
            txtFileContent = new TextBox();
            btnCreate = new Button();
            btnRead = new Button();
            btnUpdate = new Button();
            btnDelete = new Button();
            ((System.ComponentModel.ISupportInitialize)splitContainer1).BeginInit();
            splitContainer1.Panel1.SuspendLayout();
            splitContainer1.Panel2.SuspendLayout();
            splitContainer1.SuspendLayout();
            SuspendLayout();
            // 
            // txtFolderPath
            // 
            txtFolderPath.Location = new Point(12, 12);
            txtFolderPath.Name = "txtFolderPath";
            txtFolderPath.ReadOnly = true;
            txtFolderPath.Size = new Size(650, 31);
            txtFolderPath.TabIndex = 0;
            // 
            // btnBrowse
            // 
            btnBrowse.Location = new Point(670, 12);
            btnBrowse.Name = "btnBrowse";
            btnBrowse.Size = new Size(100, 32);
            btnBrowse.TabIndex = 1;
            btnBrowse.Text = "Browse...";
            btnBrowse.UseVisualStyleBackColor = true;
            btnBrowse.Click += btnBrowse_Click;
            // 
            // splitContainer1
            // 
            splitContainer1.Location = new Point(12, 50);
            splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            splitContainer1.Panel1.Controls.Add(treeViewFiles);
            // 
            // splitContainer1.Panel2
            // 
            splitContainer1.Panel2.Controls.Add(txtFileContent);
            splitContainer1.Size = new Size(760, 360);
            splitContainer1.SplitterDistance = 250;
            splitContainer1.TabIndex = 2;
            // 
            // treeViewFiles
            // 
            treeViewFiles.Dock = DockStyle.Fill;
            treeViewFiles.Location = new Point(0, 0);
            treeViewFiles.Name = "treeViewFiles";
            treeViewFiles.Size = new Size(250, 360);
            treeViewFiles.TabIndex = 0;
            // 
            // txtFileContent
            // 
            txtFileContent.Dock = DockStyle.Fill;
            txtFileContent.Location = new Point(0, 0);
            txtFileContent.Multiline = true;
            txtFileContent.Name = "txtFileContent";
            txtFileContent.ScrollBars = ScrollBars.Vertical;
            txtFileContent.Size = new Size(506, 360);
            txtFileContent.TabIndex = 0;
            // 
            // btnCreate
            // 
            btnCreate.Location = new Point(12, 420);
            btnCreate.Name = "btnCreate";
            btnCreate.Size = new Size(80, 30);
            btnCreate.TabIndex = 3;
            btnCreate.Text = "Create";
            btnCreate.UseVisualStyleBackColor = true;
            btnCreate.Click += btnCreate_Click;
            // 
            // btnRead
            // 
            btnRead.Location = new Point(110, 420);
            btnRead.Name = "btnRead";
            btnRead.Size = new Size(80, 30);
            btnRead.TabIndex = 4;
            btnRead.Text = "Read";
            btnRead.UseVisualStyleBackColor = true;
            btnRead.Click += btnRead_Click;
            // 
            // btnUpdate
            // 
            btnUpdate.Location = new Point(210, 420);
            btnUpdate.Name = "btnUpdate";
            btnUpdate.Size = new Size(80, 30);
            btnUpdate.TabIndex = 5;
            btnUpdate.Text = "Update";
            btnUpdate.UseVisualStyleBackColor = true;
            btnUpdate.Click += btnUpdate_Click;
            // 
            // btnDelete
            // 
            btnDelete.Location = new Point(310, 420);
            btnDelete.Name = "btnDelete";
            btnDelete.Size = new Size(80, 30);
            btnDelete.TabIndex = 6;
            btnDelete.Text = "Delete";
            btnDelete.UseVisualStyleBackColor = true;
            btnDelete.Click += btnDelete_Click;
            // 
            // Form1
            // 
            ClientSize = new Size(784, 461);
            Controls.Add(btnDelete);
            Controls.Add(btnUpdate);
            Controls.Add(btnRead);
            Controls.Add(btnCreate);
            Controls.Add(splitContainer1);
            Controls.Add(btnBrowse);
            Controls.Add(txtFolderPath);
            Name = "Form1";
            Text = "Simple File Manager";
            splitContainer1.Panel1.ResumeLayout(false);
            splitContainer1.Panel2.ResumeLayout(false);
            splitContainer1.Panel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)splitContainer1).EndInit();
            splitContainer1.ResumeLayout(false);
            ResumeLayout(false);
            PerformLayout();
        }
    }
}
