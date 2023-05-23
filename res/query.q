{
  aio_specimen(
    filter: [
      {
        field: "projectReferenceIds"
        operator: CONTAINS
        value: "1HEYEW7GMUKWIQW37BO"
      }
      ,{field:"4D4RGMXTPBL0YAY5UGJ"
       operator:EQ
       value:"1"
      }
    ]
  )
  {
    referenceId
    specimenType {
      name
    }
    cRID {
      symbol
    }
    images{
      featureType {referenceId}
      url
    }
    annotations(featureTypes:["Z8BZR1LDP9G4I97O394", "DW0F0S320VR4NX0DBPT", "17Q8WESJFABC8QV7O69", "O267WEUPC2MMK3MI7ZY"])
    {
      featureType
      {
        referenceId
        title
        description
      }
      modality { name }
      taxons { description, symbol }
    }
  }
}
