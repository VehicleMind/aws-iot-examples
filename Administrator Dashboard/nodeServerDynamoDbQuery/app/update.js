var AWS = require("aws-sdk");
var awsConfig = {
    "region": "ap-south-1",
    "endpoint": "http://dynamodb.ap-south-1.amazonaws.com",
    "accessKeyId": "AKIAJZMYM76N62LM3QDQ", "secretAccessKey": "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"
};
AWS.config.update(awsConfig);

var docClient = new AWS.DynamoDB.DocumentClient();

var modify = function () {


    var params = {
        TableName: "sampletable",
        Key: {
            "name": "John Mayo-Smith",
            "city": "New York"
        },
        UpdateExpression: "set food= :newFood",
        ExpressionAttributeValues: {
            ":newFood": "greninja"
        },
        ReturnValues: "UPDATED_NEW"

    };
    docClient.update(params, function (err, data) {

        if (err) {
            console.log("users::update::error - " + JSON.stringify(err, null, 2));
        } else {
            console.log("users::update::success "+JSON.stringify(data) );
        }
    });
}
module.exports.modify = modify
