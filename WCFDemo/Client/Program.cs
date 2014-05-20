﻿using System;
using System.ServiceModel;
using Artech.WcfServices.Contracts;

namespace Artech.WcfServices.Client
{
    class Program
    {
        static void Main(string[] args)
        {
            using (ChannelFactory<ICalculator> channelFactory = new ChannelFactory<ICalculator>("CalculatorService"))
            {
                ICalculator proxy = channelFactory.CreateChannel();
                using (proxy as IDisposable)
                {
                    Console.WriteLine("x + y = {2} when x = {0} and y = {1}", 1, 2, proxy.Add(1, 2));
                    Console.WriteLine("x - y = {2} when x = {0} and y = {1}", 1, 2, proxy.Subtract(1, 2));
                    Console.WriteLine("x * y = {2} when x = {0} and y = {1}", 1, 2, proxy.Multiply(1, 2));
                    Console.WriteLine("x / y = {2} when x = {0} and y = {1}", 1, 2, proxy.Divide(1, 2));
                
                }
                Console.Read();
            }
        }
    }
}

