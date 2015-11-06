//
//  ViewController.swift
//  controlRobot
//
//  Created by Axel Goetz on 10/06/15.
//  Copyright (c) 2015 Axel Goetz. All rights reserved.
//

import UIKit

class ViewController: UIViewController, UITextFieldDelegate {

    @IBOutlet var textField: UITextField!
    @IBOutlet var speedTextField: UITextField!
    
    var inp :NSInputStream?
    var out :NSOutputStream?
    
    var inputStream: NSInputStream!
    var outputStream: NSOutputStream!
    
    var temp: MFLSwiftJoystickImplementation!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        textField.delegate = self
        speedTextField.delegate = self
        
        let screenSize: CGRect = UIScreen.mainScreen().bounds
        let frame = CGRect(x: 50, y: 300, width: screenSize.width-100, height: screenSize.width - 100)
        
        temp = MFLSwiftJoystickImplementation(frame: frame, outputStream: outputStream)
        temp.setupWithThumbAndBackgroundImages(UIImage(named: "joy_thumb.png")!, bgImage: UIImage(named: "stick_base.png")!)
        self.view.addSubview(temp)
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    func textFieldShouldReturn(textField: UITextField) -> Bool // called when 'return' key pressed. return NO to ignore.
    {
        textField.resignFirstResponder()
        return true;
    }

    @IBAction func goButtonPressed(sender: AnyObject) {
        textFieldShouldReturn(textField)
        if inputStream != nil && outputStream != nil {
            inputStream.close()
            outputStream.close()
        }
        let addr = textField.text
        let port = 55443

        NSStream.getStreamsToHostWithName(addr, port: port, inputStream: &inp, outputStream: &out)
        
        inputStream = inp!
        outputStream = out!
        
        inputStream.open()
        outputStream.open()
        
        
        temp.setOutputStream1(outputStream)
    }
    
    
    @IBAction func speedButtonPressed(sender: AnyObject) {
        textFieldShouldReturn(speedTextField)
        
        if outputStream == nil {
            let actionSheetController: UIAlertController = UIAlertController(title: "WARNING", message: "First select an IP", preferredStyle: .ActionSheet)
            
            //Create and add the Cancel action
            let cancelAction: UIAlertAction = UIAlertAction(title: "Ok", style: .Cancel) { action -> Void in
                //Just dismiss the action sheet
            }
            actionSheetController.addAction(cancelAction)
            
            //Present the AlertController
            self.presentViewController(actionSheetController, animated: true, completion: nil)
        }
        
        if let maxSpeed = speedTextField.text.toInt() {
            if(maxSpeed > 127 || maxSpeed < 0) {
                let actionSheetController: UIAlertController = UIAlertController(title: "WARNING", message: "Speed too low of too high!", preferredStyle: .ActionSheet)
                
                //Create and add the Cancel action
                let cancelAction: UIAlertAction = UIAlertAction(title: "Ok", style: .Cancel) { action -> Void in
                    //Just dismiss the action sheet
                }
                actionSheetController.addAction(cancelAction)
                
                //Present the AlertController
                self.presentViewController(actionSheetController, animated: true, completion: nil)
                temp.setSpeed(50)
            }
            else {
                temp.setSpeed(maxSpeed)
            }
        }
        else {
            let actionSheetController: UIAlertController = UIAlertController(title: "WARNING", message: "Not a valid input!", preferredStyle: .ActionSheet)
            
            //Create and add the Cancel action
            let cancelAction: UIAlertAction = UIAlertAction(title: "Ok", style: .Cancel) { action -> Void in
                //Just dismiss the action sheet
            }
            actionSheetController.addAction(cancelAction)
            
            //Present the AlertController
            self.presentViewController(actionSheetController, animated: true, completion: nil)
        }
    }
    
}

