using System;
using System.IO;
using System.Collections;
using System.Text.RegularExpressions;
using System.Text;
using System.Runtime.Serialization.Json;

namespace CrawlerCommon
{
	/// <summary>
	/// Summary description for SortTree.
	/// </summary>
	public class CrawlerBinaryTree<T> where T : IComparable
	{
        public CrawlerBinaryTreeNode<T> root_;
		public int count_;
		public bool isModified_;

		public CrawlerBinaryTree()
		{
            Clear();
		}
		public void Clear()
		{
			root_ = null;
			count_ = 0;
			isModified_ = false;
		}

        // make sure that t is inherit from IComparable!
        public CrawlerBinaryTreeNode<T> AddTreeNode(T t)
		{
            CrawlerBinaryTreeNode<T> node;
			if(root_ == null)
			{
                root_ = new CrawlerBinaryTreeNode<T>(t, 0);
				node = root_;
			}
			else	
			{
				node = root_;
				while(true)
				{
                    if (node.data_.CompareTo(t) == 0)
					{
						node.count_++;
						return node;
					}
                    else if (node.data_.CompareTo(t) > 0)
					{	
                        // add the node at the small branch
						if(node.small_ == null)
						{
                            node.small_ = new CrawlerBinaryTreeNode<T>(t);
							node.small_.parent_ = node;
							node = node.small_;
							break;
						}
						node = node.small_;
					}
                    else if (node.data_.CompareTo(t) < 0)
					{	// add the node at the great branch
						if(node.great_ == null)
						{
                            node.great_ = new CrawlerBinaryTreeNode<T>(t);
							node.great_.parent_ = node;
							node = node.great_;
							break;
						}
						node = node.great_;
					}
				}	
			}
			node.nodeID_ = this.count_;
            this.count_++;
			node.count_++;
			isModified_ = true;
			
			return node;
		}
		
		public void WriteToFile(string fileName, System.Text.Encoding code)
		{
			FileStream stream = new FileStream(fileName, FileMode.Create, FileAccess.Write, FileShare.None);
			StreamWriter writer = new StreamWriter(stream, code);
			
			BitArray bitArray = new BitArray(this.count_, false);
            CrawlerBinaryTreeNode<T> node = this.root_;
			int nCount = this.count_;
			while(nCount > 0)
			{
                if (node.small_ != null && bitArray.Get(node.small_.nodeID_) == false)
                {
                    node = node.small_;
                }
                else if (bitArray.Get(node.nodeID_) == false)
                {
                    WriteNodeToFile(node, writer, bitArray, ref nCount);
                }
                else if (node.great_ != null && bitArray.Get(node.great_.nodeID_) == false)
                {
                    node = node.great_;
                }
                else
                {
                    if (bitArray.Get(node.nodeID_) == false)
                        WriteNodeToFile(node, writer, bitArray, ref nCount);
                    node = node.parent_;
                }
			}
			writer.Close();
			stream.Close();
			
			isModified_ = false;
		}

        // output one tree node to file
        private bool WriteNodeToFile(CrawlerBinaryTreeNode<T> node, StreamWriter writer, BitArray bits, ref int nCount)
        {
            if (node == null || bits.Get(node.nodeID_) == true)
                return false;

            writer.WriteLine(node.ToJsonString());

            bits.Set(node.nodeID_, true);
            nCount--;

            return true;
        }
        // find tree node from given data t
        public CrawlerBinaryTreeNode<T> FindTreeNodeByData(T t)
        {
            CrawlerBinaryTreeNode<T> node = null;
            if (root_ == null)
            {
                return node; // no one node exits, return null
            }
            else
            {
                node = root_;
                while (true)
                {
                    if (node.data_.CompareTo(t) == 0)
                    {
                        // find what i want, return it
                        return node;
                    }
                    else if (node.data_.CompareTo(t) > 0)
                    {
                        // traverse the smaller branch
                        if (node.small_ == null)
                        {
                            // no one node with data t exits, return null
                            return null;
                        }
                        node = node.small_;
                    }
                    else if (node.data_.CompareTo(t) < 0)
                    {	
                        // traverse the greater branch
                        if (node.great_ == null)
                        {
                            // no one node with data t exits, return null
                            return null;
                        }
                        node = node.great_;
                    }
                }
            }
        }
	}

    public class CrawlerBinaryTreeNode<T> : CrawlerFileEntity where T : IComparable
	{
        public CrawlerBinaryTreeNode<T> parent_; // parent
        public CrawlerBinaryTreeNode<T> small_; // left child
        public CrawlerBinaryTreeNode<T> great_; // right child
		public T data_; // data
        public int count_; // 
		public int nodeID_;

        // default constructor, ensure that T inherit IComparable
        public CrawlerBinaryTreeNode(T t, int nodeId = -1)
        {
            this.dataTypeName_ = "CrawlerBinaryTreeNode";
            this.version_ = 1;
            this.filePath_ = "CrawlerBinaryTreeNode.json";
            data_ = t;
            nodeID_ = nodeId;
        }

        // generate an instance from Json string
        override public void FromJsonString(string jsonString)
        {
            var mStream = new MemoryStream(Encoding.Default.GetBytes(jsonString));
            var serializer = new DataContractJsonSerializer(typeof(CrawlerBinaryTreeNode<T>));
            CopyFrom((CrawlerBinaryTreeNode<T>)serializer.ReadObject(mStream));
        }

        // translate 'this' to Json string
        override public string ToJsonString()
        {
            try
            {
                var serializer = new DataContractJsonSerializer(typeof(CrawlerBinaryTreeNode<T>));
                var stream = new MemoryStream();
                serializer.WriteObject(stream, this);
                byte[] dataBytes = new byte[stream.Length];
                stream.Position = 0;
                stream.Read(dataBytes, 0, (int)stream.Length);
                string jsonString = Encoding.UTF8.GetString(dataBytes);
                return jsonString;
            }
            catch (System.Exception ex)
            {
                // TODO: error handling.
                return "";
            }
        }

        // copy from
        public CrawlerBinaryTreeNode<T> CopyFrom(CrawlerBinaryTreeNode<T> other)
        {
            base.CopyFrom((CrawlerCommon.CrawlerFileEntity)other);
            data_ = other.data_;
            count_ = other.count_;
            nodeID_ = other.nodeID_;
            return this;
        }
	}
}
