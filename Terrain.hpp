#include <cmath>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class Terrain {

private:

	int terrainDimension;

	//Are the coordinates passed in within the heightmap
	bool isValidCoordinate(unsigned int row, unsigned int column) {

		if (row > this->terrainDimension - 1 || column > this->terrainDimension - 1) {
			return false;
		}
		else {
			return true;
		}
	}

public:
	//Constructor
	Terrain(unsigned int dimension) {

		//The dimension should be a power of 2 plus 1
		double logValue = log2(dimension - 1);
		long logValueLong = logValue;
		if (logValueLong == logValue) {
			this->heightMap = std::shared_ptr<std::vector<float>>(new std::vector<float>(dimension * dimension, 0.0));
			this->terrainDimension = dimension;
		}
		else {
			throw std::invalid_argument("Terrain dimension must be a power of 2 plus 1.");
		}
	}

	//Destructor
	~Terrain() {
		this->heightMap.reset();
	}

	//Return the terrain that was created
	std::vector<float> getTerrain() {
		return *this->heightMap;
	}

	//Get the terrain height at a specific row and column position
	float getHeightAt(unsigned int row, unsigned int column) {

		if (isValidCoordinate(row, column)) {
			return this->heightMap->at(row * this->terrainDimension + column);
		}
		else {
			std::stringstream errorMessage;
			errorMessage << "Invalid row " << row << " or column " << column << " size.";
			throw std::invalid_argument(errorMessage.str());
		}
	}

	//Set the terrain height at a specific row and column position
	void setHeightAt(unsigned int row, unsigned int column, float height) {

		if (isValidCoordinate(row, column)) {

			this->heightMap->at(row * this->terrainDimension + column) = height;
		}
		else {
			std::stringstream errorMessage;
			errorMessage << "Invalid row " << row << " or column " << column << " size.";
			throw std::invalid_argument(errorMessage.str());
		}
	}

	//Get the dimension of the terrain
	unsigned int getTerrainDimension() {
		return this->terrainDimension;
	}

protected:

	std::shared_ptr<std::vector<GLfloat>> heightMap;

	//This method needs to be implemented by the child class
	virtual void makeTerrain() = 0;

	//Convert row columns to offset
	unsigned int getLocationOffset(unsigned int row, unsigned int column) {
		return row * this->terrainDimension + column;
	}

	//Check if the offset passed in is within the heightmap
	bool isValidOffset(unsigned int offset) {
		if (offset < this->terrainDimension * this->terrainDimension) {
			return true;
		}
		else {
			return false;
		}
	}

	//Get the terrain height at a specific row and column position
	float getHeightAt(unsigned int offset) {

		unsigned int row = offset / this->terrainDimension;
		unsigned int column = offset % this->terrainDimension;
		return getHeightAt(row, column);

	}

	//Set the terrain height at a specific offset
	void setHeightAt(unsigned int offset, float height) {

		unsigned int row = offset / this->terrainDimension;
		unsigned int column = offset % this->terrainDimension;
		setHeightAt(row, column, height);

	}

};

//This terrain is built by particles being deposited. The deposition starts at one point and then based on a random number, 
//the next particle is deposited on one of the four neighboring locations.
class ParticleDepositionTerrain : public Terrain {

private:
	
	//Number of iterations
	const unsigned int NUMBER_OF_ITERATIONS = 100000;

	//This is the size of each particle deposited on the terrain
	const float PARTICLE_SIZE = 0.01;

protected:


	//Start location for terrain generation
	char startLocation;

	//Find first location in current row
	unsigned int firstLocationInCurrentRow(unsigned int currentOffset) {

		unsigned int currentRow = currentOffset / getTerrainDimension();
		return currentRow * Terrain::getTerrainDimension();

	}

	//Find last location in current row
	unsigned int lastLocationInCurrentRow(unsigned int currentOffset) {

		unsigned int currentRow = currentOffset / Terrain::getTerrainDimension();
		return currentRow * Terrain::getTerrainDimension() + getTerrainDimension() - 1;

	}
	//Find the point on the terrain where deposition should start
	unsigned int seedParticleDeposition() {

		if (this->startLocation == 'r' || this->startLocation == 'R') {
			std::default_random_engine randomNumberGenerator;
			std::uniform_int_distribution<int> distribution(0, getTerrainDimension() * getTerrainDimension() - 1);
			return distribution(randomNumberGenerator);
		}
		else {
			return (getTerrainDimension() * getTerrainDimension()) / 2;
		}

	}

	//Deposit a particle at a specified location and increase the terrain height there
	void depositParticle(unsigned int offset) {
		setHeightAt(offset, getHeightAt(offset) + PARTICLE_SIZE);
	}

public:

	//Constructor
	ParticleDepositionTerrain(unsigned int dimension, char startLocation) : Terrain (dimension) {
		this->startLocation = startLocation;
	}

	//Make the terrain based on particle deposition 
	void makeTerrain() {

		//Find the location to start depositing particles
		unsigned int startingLocation = seedParticleDeposition(), currentLocation = startingLocation, nextLocation;

		//Random number generator with four equally likely outcomes
		std::default_random_engine randomNumberGenerator;
		std::discrete_distribution<int> distribution{ 0.25, 0.25, 0.25, 0.25 };

		//Deposit particles based on the number of iterations specified
		for (auto counter = 0; counter < NUMBER_OF_ITERATIONS; ++counter) {

			int randomNumber = distribution(randomNumberGenerator);
			//std::cout << "Random number is " << randomNumber << std::endl;

			switch (randomNumber) {

				//Neighbor to the right
			case 0:
				if (isValidOffset(currentLocation + 1)) {
					nextLocation = currentLocation + 1;
				}
				else {//wrap around
					nextLocation = firstLocationInCurrentRow(currentLocation);
				}
				break;

				//Neighbor to the left
			case 1:
				if (isValidOffset(currentLocation - 1)) {
					nextLocation = currentLocation - 1;
				}
				else {
					nextLocation = lastLocationInCurrentRow(currentLocation);
				}
				break;

				//Neighbor above
			case 2:
				{
					  long neighborAbove = currentLocation - getTerrainDimension();
					  if (neighborAbove >= 0 && isValidOffset(neighborAbove)) {
						  nextLocation = (unsigned int)neighborAbove;
					  }
					  else {//wrap to the corresponding element in last row
						  nextLocation = getLocationOffset(getTerrainDimension() - 1, currentLocation);
					  }
				}
				break;

				//Neighbor below
			case 3:
				{
					  unsigned int neighborBelow = currentLocation + getTerrainDimension();
					  if (isValidOffset(neighborBelow)) {
						  nextLocation = neighborBelow;
					  }
					  else {//wrap to the corresponding element in first row
						  unsigned int columnOfLastRow = currentLocation % getTerrainDimension();
						  nextLocation = getLocationOffset(0, columnOfLastRow);
					  }
				}
				break;

			}

			//std::cout << "Deposit particle at " << nextLocation << std::endl;
			depositParticle(nextLocation);
			currentLocation = nextLocation;
		}
	}


};

//This terrain is built by particles being deposited. The deposition starts at one point and then based on a random number, 
//the next particle is deposited on one of the four neighboring locations. If the location the particle is deposited in is higher than
//any of its neighbors, then the particle rolls down.
class RollDownParticleDepositionTerrain : public ParticleDepositionTerrain {

private:

	//Number of iterations
	const unsigned int NUMBER_OF_ITERATIONS = 100000;

	//This is the size of each particle deposited on the terrain
	const float PARTICLE_SIZE = 0.01;

	//If right, left, upper or lower neighbor is lower then the particle will roll down to that location
	unsigned int getRollDownLocation(unsigned int currentLocation) {

		//Check neighbor on the right
		unsigned int neighbor;
		if (isValidOffset(currentLocation + 1)) {
			neighbor = currentLocation + 1;
		}
		else {//wrap around
			neighbor = firstLocationInCurrentRow(currentLocation);
		}
		//Roll down if lower
		if (getHeightAt(neighbor) < getHeightAt(currentLocation)) {
			return neighbor;
		}

		//Check neighbor on the left
		if (isValidOffset(currentLocation - 1)) {
			neighbor = currentLocation - 1;
		}
		else {
			neighbor = lastLocationInCurrentRow(currentLocation);
		}
		//Roll down if lower
		if (getHeightAt(neighbor) < getHeightAt(currentLocation)) {
			return neighbor;
		}

		//Check neighbor above
		long neighborAbove = currentLocation - getTerrainDimension();
		if (neighborAbove >= 0 && isValidOffset(neighborAbove)) {
			neighbor = (unsigned int)neighborAbove;
		}
		else {//wrap to the corresponding element in last row
			neighbor = getLocationOffset(getTerrainDimension() - 1, currentLocation);
		}
		//Roll down if lower
		if (getHeightAt(neighbor) < getHeightAt(currentLocation)) {
			return neighbor;
		}

		//Check neighbor below
		unsigned int neighborBelow = currentLocation + getTerrainDimension();
		if (isValidOffset(neighborBelow)) {
			neighbor = neighborBelow;
		}
		else {//wrap to the corresponding element in first row
			unsigned int columnOfLastRow = currentLocation % getTerrainDimension();
			neighbor = getLocationOffset(0, columnOfLastRow);
		}
		//Roll down if lower
		if (getHeightAt(neighbor) < getHeightAt(currentLocation)) {
			return neighbor;
		}

		return currentLocation;
	}

public:
	//Constructor
	RollDownParticleDepositionTerrain(unsigned int dimension, char startLocation) : ParticleDepositionTerrain(dimension, startLocation) {
	}

	//Make the terrain based on particle deposition 
	void makeTerrain() {

		//Find the location to start depositing particles
		unsigned int startingLocation = seedParticleDeposition(), currentLocation = startingLocation, nextLocation, rollDownLocation;

		//Random number generator with four equally likely outcomes
		std::default_random_engine randomNumberGenerator;
		std::discrete_distribution<int> distribution{ 0.25, 0.25, 0.25, 0.25 };

		//Deposit particles based on the number of iterations specified
		for (auto counter = 0; counter < NUMBER_OF_ITERATIONS; ++counter) {

			int randomNumber = distribution(randomNumberGenerator);
			//std::cout << "Random number is " << randomNumber << std::endl;

			switch (randomNumber) {

				//Neighbor to the right
			case 0:
				if (isValidOffset(currentLocation + 1)) {
					nextLocation = currentLocation + 1;
				}
				else {//wrap around
					nextLocation = firstLocationInCurrentRow(currentLocation);
				}
				break;

				//Neighbor to the left
			case 1:
				if (isValidOffset(currentLocation - 1)) {
					nextLocation = currentLocation - 1;
				}
				else {
					nextLocation = lastLocationInCurrentRow(currentLocation);
				}
				break;

				//Neighbor above
			case 2:
			{
					  long neighborAbove = currentLocation - getTerrainDimension();
					  if (neighborAbove >= 0 && isValidOffset(neighborAbove)) {
						  nextLocation = (unsigned int)neighborAbove;
					  }
					  else {//wrap to the corresponding element in last row
						  nextLocation = getLocationOffset(getTerrainDimension() - 1, currentLocation);
					  }
			}
				break;

				//Neighbor below
			case 3:
			{
					  unsigned int neighborBelow = currentLocation + getTerrainDimension();
					  if (isValidOffset(neighborBelow)) {
						  nextLocation = neighborBelow;
					  }
					  else {//wrap to the corresponding element in first row
						  unsigned int columnOfLastRow = currentLocation % getTerrainDimension();
						  nextLocation = getLocationOffset(0, columnOfLastRow);
					  }
			}
				break;

			}

			//std::cout << "Deposit particle at " << nextLocation << std::endl;
			depositParticle(getRollDownLocation(nextLocation));
			currentLocation = nextLocation;
		}
	}


}; 

//This terrain is built by generating successive faults from one side to the opposite side. The fault distorts the terrain
class FaultTerrain : public Terrain {

private:

	//Vector containing pairs of offsets for the two ends of fault lines
	std::vector<unsigned int> faultLineEnds;

	//Number of iterations
	const unsigned int NUMBER_OF_ITERATIONS = 300;

	void generateFaults() {

		//Random number generator for selecting the left or top edge at random
		std::default_random_engine randomTerrainEdgeSelector;
		std::discrete_distribution<int> equallyLikelyDistribution{ 0.5, 0.5 };

		//Random number generator for selecting one cell from an edge
		std::default_random_engine randomEdgeCellGenerator;
		std::uniform_int_distribution<int> edgeCellDistribution(0, getTerrainDimension() - 1);

		//Generate faults in the terrain
		unsigned int randomEdge = 0, randomEdgeCell1 = 0, randomEdgeCell2 = 0, faultLineEnd1 = 0, faultLineEnd2 = 0;
		for (auto counter = 0; counter < NUMBER_OF_ITERATIONS; ++counter) {

			//Select the left or top edge at random
			randomEdge = equallyLikelyDistribution(randomTerrainEdgeSelector);

			//Select the cells at the two ends of the fault
			randomEdgeCell1 = edgeCellDistribution(randomEdgeCellGenerator);
			randomEdgeCell2 = edgeCellDistribution(randomEdgeCellGenerator);

			switch (randomEdge) {

				//Left edge to right edge
			case 0:
				faultLineEnd1 = getTerrainDimension() * randomEdgeCell1;
				faultLineEnd2 = getTerrainDimension() * (randomEdgeCell2 + 1) - 1;
				break;

				//Top edge to bottom edge
			case 1:
				faultLineEnd1 = randomEdgeCell1;
				faultLineEnd2 = getTerrainDimension() * (getTerrainDimension() - 1) + randomEdgeCell2;
				break;

			}

			this->faultLineEnds.push_back(faultLineEnd1);
			this->faultLineEnds.push_back(faultLineEnd2);
		}
	}

public:
	//Constructor
	FaultTerrain(int dimension) : Terrain(dimension) {
		generateFaults();
	}

protected:

	unsigned int getFaultCount() {
		return this->faultLineEnds.size() / 2;
	}

	unsigned int getFaultLineEnd(unsigned int offset) {
		return this->faultLineEnds.at(offset);
	}

	//Should the passed in point be raised or not
	bool raisePoint(unsigned int faultLineEnd1, unsigned int faultLineEnd2, unsigned int point) {

		unsigned int faultLineEnd1Row = 0, faultLineEnd1Column = 0, faultLineEnd2Row = 0, faultLineEnd2Column = 0, pointRow, pointColumn = 0;

		faultLineEnd1Row = faultLineEnd1 / getTerrainDimension();
		faultLineEnd1Column = faultLineEnd1 % getTerrainDimension();

		faultLineEnd2Row = faultLineEnd2 / getTerrainDimension();
		faultLineEnd2Column = faultLineEnd2 % getTerrainDimension();

		pointRow = point / getTerrainDimension();
		pointColumn = point % getTerrainDimension();

		int pointLocation = (faultLineEnd2Column - faultLineEnd1Column) * (pointRow - faultLineEnd1Row) -
			(faultLineEnd2Row - faultLineEnd1Row) * (pointColumn - faultLineEnd1Column);

		if (pointLocation > 0) {
			return true;
		}
		else {
			return false;
		}
		
	}

	//This method needs to be implemented by the child class
	virtual void makeTerrain() = 0;

};

//This terrain is built by generating successive faults from one side to the opposite side. The fault distorts the terrain by pushing one side up.
class StepFaultTerrain : public FaultTerrain {

private:
	//This is the size of each particle deposited on the terrain
	const float STEP_SIZE = 0.004;

public:
	//Constructor
	StepFaultTerrain(int dimension) : FaultTerrain(dimension) {
	}

	void makeTerrain() {

		unsigned int numberOfFaultLineEnds = 2 * getFaultCount(), totalNumberOfPoints = getTerrainDimension() * getTerrainDimension();
		//Retrieve the fault line and create faults in terrain
		for (auto faultCounter = 0; faultCounter < numberOfFaultLineEnds; faultCounter += 2) {

			//Loop through each point in terrain and raise it if needed
			for (auto pointCounter = 0; pointCounter < totalNumberOfPoints; ++pointCounter) {

				if (raisePoint(getFaultLineEnd(faultCounter), getFaultLineEnd(faultCounter + 1), pointCounter)) {
					setHeightAt(pointCounter, getHeightAt(pointCounter) + STEP_SIZE);

				}
			}
		}
	}

};

//This terrain is built by generating successive faults from one side to the opposite side. The fault distorts the terrain
class BumpTerrain : public Terrain {

private:

	double bumpDiameter = 0;

	const int NUMBER_OF_ITERATIONS = 1000;

	//Vector containing bump centers
	std::vector<unsigned int> bumpCenters;

	//Randomly generate bump locations throughout the terrain
	void generateBumpCenters() {

		//Random number generator for selecting a cell in the terrain
		std::default_random_engine randomCellSelector;
		std::uniform_int_distribution<int> randomCellDistribution(0, getTerrainDimension() * getTerrainDimension() - 1);

		unsigned int bumpLocation;
		for (auto bumpCounter = 0; bumpCounter < NUMBER_OF_ITERATIONS; ++bumpCounter) {

			//Generate and store bump centers
			bumpLocation = randomCellDistribution(randomCellSelector);
			bumpCenters.push_back(bumpLocation);

		}

	}

	//Set the bump diameter to be 5% of the terrain dimension
	void setBumpDiameter() {

		bumpDiameter = getTerrainDimension() * 0.05;

	}

	//Create a cosine bump at this location
	void createBump(unsigned int bumpCenter) {

		//Check if bump is within the terrain
		long bumpTop = 0, bumpBottom = 0, bumpLeft = 0, bumpRight = 0;
		bumpTop = bumpCenter - (getTerrainDimension() * bumpDiameter / 2) - 15 * getTerrainDimension();
		bumpBottom = bumpCenter + (getTerrainDimension() * bumpDiameter / 2) + 15 * getTerrainDimension();
		bumpLeft = bumpCenter - bumpDiameter / 2 - 15;
		bumpRight = bumpCenter + bumpDiameter / 2 + 15;

		//Quit if bump top is outside the terrain
		if (bumpTop < 0) {
			return;
		}

		//Quit if bump bottom is outside the terrain
		if (!isValidOffset((unsigned int)bumpBottom)) {
			return;
		}

		//Quit if bump left is outside the terrain or not on same row
		unsigned int bumpCenterRow, bumpLeftRow, bumpRightRow, bumpTopRow = 0, bumpBottomRow = 0, bumpLeftColumn = 0, bumpRightColumn = 0, bumpCenterColumn = 0, cellLocation = 0;
		if (bumpLeft < 0 || bumpRight < 0) {
			return;
		}
		if (!isValidOffset((unsigned int)bumpLeft) || !isValidOffset((unsigned int)bumpRight)) {
			return;
		}

		bumpCenterRow = bumpCenter / getTerrainDimension();
		bumpLeftRow = bumpLeft / getTerrainDimension();
		bumpRightRow = bumpRight / getTerrainDimension();

		if (bumpCenterRow != bumpLeftRow || bumpCenterRow != bumpRightRow) {
			return;
		}

		//Loop through every cell in the square encircling the bump
		bumpTopRow = bumpTop / getTerrainDimension();
		bumpBottomRow = bumpBottom / getTerrainDimension();
		bumpLeftColumn = bumpLeft % getTerrainDimension();
		bumpRightColumn = bumpRight % getTerrainDimension();
		bumpCenterColumn = bumpCenter % getTerrainDimension();
		int verticalDistance = 0, horizontalDistance = 0;
		double distanceFromCenter = 0;
		double cellLocationAngleEquivalent = 0;
		float cellHeight = 0.0;
		for (auto rowCounter = bumpTopRow; rowCounter <= bumpBottomRow; ++rowCounter) {
			for (auto columnCounter = bumpLeftColumn; columnCounter <= bumpRightColumn; ++columnCounter) {
				//Check if this cell is within the dump diamet er
				verticalDistance = rowCounter - bumpCenterRow;
				horizontalDistance = columnCounter - bumpCenterColumn;
				distanceFromCenter = sqrt(pow(verticalDistance, 2) + pow(horizontalDistance, 2));
				//If cell is within the bump diameter raise it by cosine amount
				if (distanceFromCenter <= bumpDiameter) {
					cellLocationAngleEquivalent = (distanceFromCenter / bumpDiameter) * M_PI / 2.0;
					cellHeight = cos(cellLocationAngleEquivalent) / 35.0;
					cellLocation = getLocationOffset(columnCounter, rowCounter);
					setHeightAt(cellLocation, getHeightAt(cellLocation) + cellHeight);
				}
			}

		}
	}

public:
	//Constructor
	BumpTerrain(int dimension) : Terrain(dimension) {
		generateBumpCenters();
		setBumpDiameter();
	}

	void makeTerrain() {

		//Loop through the bump locations and create cosine bumps
		unsigned int numberOfBumps = bumpCenters.size();
		for (auto bumpCounter = 0; bumpCounter < numberOfBumps; ++bumpCounter) {

			//Create a bump at this location
			createBump(bumpCenters.at(bumpCounter));

		}

	}
};

//This terrain is built by the square-diamond method
class SquareDiamondTerrain : public Terrain {

private:

	//Terrain roughness
	const float TERRAIN_ROUGHNESS = 0.2;

	//Random number generator for selecting a cell in the terrain
	std::default_random_engine roughnessGenerator;
	std::normal_distribution<float> distribution = std::normal_distribution<GLfloat>(0.0, 0.7);

	//This method takes the top left and bottom right coordinates of a terrain and subdivides it using the square-diamond method. 
	void subDivide(unsigned int topLeftRow, unsigned int topLeftColumn, unsigned int bottomRightRow, unsigned int bottomRightColumn, float roughness) {

		//Return if terrain cannot be subdivided
		if (!(bottomRightColumn - topLeftColumn > 1)) {
			return;
		}

		//Find the diamond vertex and compute its height as the average of the height of the four vertices passed in
		unsigned int diamondVertexRow = (bottomRightRow + topLeftRow) / 2, diamondVertexColumn = (bottomRightColumn + topLeftColumn) / 2;
		float diamondVertexHeight = (getHeightAt(getLocationOffset(topLeftRow, topLeftColumn)) +
			                         getHeightAt(getLocationOffset(bottomRightRow, bottomRightColumn)) +
			                         getHeightAt(getLocationOffset(bottomRightRow, topLeftColumn)) +
									 getHeightAt(getLocationOffset(topLeftRow, bottomRightColumn))) / 4.0;

		//Generate noise and add it to the diamond vertex height
		float noise = distribution(roughnessGenerator) * roughness;
		diamondVertexHeight += noise;
		setHeightAt(getLocationOffset(diamondVertexRow, diamondVertexColumn), diamondVertexHeight);

		//Find the square vertices
		unsigned int squareVertex1Row = (topLeftRow + bottomRightRow) / 2;
		unsigned int squareVertex1Column = topLeftColumn;
		unsigned int squareVertex2Row = bottomRightRow;
		unsigned int squareVertex2Column = (topLeftColumn + bottomRightColumn) / 2;
		unsigned int squareVertex3Row = squareVertex1Row;
		unsigned int squareVertex3Column = bottomRightColumn;
		unsigned int squareVertex4Row = topLeftRow;
		unsigned int squareVertex4Column = squareVertex2Column;

		//Compute the height at the square vertices
		float squareVertex1Height = (2 * diamondVertexHeight + 
			                         getHeightAt(getLocationOffset(topLeftRow, topLeftColumn)) + 
									 getHeightAt(getLocationOffset(bottomRightRow, topLeftColumn))) / 4.0;
		noise = distribution(roughnessGenerator) * roughness;
		squareVertex1Height += noise;
		setHeightAt(getLocationOffset(squareVertex1Row, squareVertex1Column), squareVertex1Height);

		float squareVertex2Height = (2 * diamondVertexHeight +
			                         getHeightAt(getLocationOffset(bottomRightRow, topLeftColumn)) +
									 getHeightAt(getLocationOffset(bottomRightRow, bottomRightColumn))) / 4.0;
		noise = distribution(roughnessGenerator) * roughness;
		squareVertex2Height += noise;
		setHeightAt(getLocationOffset(squareVertex2Row, squareVertex2Column), squareVertex2Height);

		float squareVertex3Height = (2 * diamondVertexHeight +
			                         getHeightAt(getLocationOffset(bottomRightRow, bottomRightColumn)) +
			                         getHeightAt(getLocationOffset(topLeftRow, bottomRightColumn))) / 4.0;
		noise = distribution(roughnessGenerator) * roughness;
		squareVertex3Height += noise;
		setHeightAt(getLocationOffset(squareVertex3Row, squareVertex3Column), squareVertex3Height);

		float squareVertex4Height = (2 * diamondVertexHeight +
			                         getHeightAt(getLocationOffset(topLeftRow, bottomRightColumn)) +
			                         getHeightAt(getLocationOffset(topLeftRow, topLeftColumn))) / 4.0;
		noise = distribution(roughnessGenerator) * roughness;
		squareVertex4Height += noise;
		setHeightAt(getLocationOffset(squareVertex4Row, squareVertex4Column), squareVertex4Height);

		//Call this method recursively for each of the four squares
		subDivide(topLeftRow, topLeftColumn, diamondVertexRow, diamondVertexColumn, roughness / 2.0);
		subDivide(squareVertex1Row, squareVertex1Column, squareVertex2Row, squareVertex2Column, roughness / 2.0);
		subDivide(diamondVertexRow, diamondVertexColumn, bottomRightRow, bottomRightColumn, roughness / 2.0);
		subDivide(squareVertex4Row, squareVertex4Column, squareVertex3Row, squareVertex3Column, roughness / 2.0);

	}



public:

	//Constructor
	SquareDiamondTerrain(int dimension) : Terrain(dimension) {
	}

	void makeTerrain() {
		subDivide(0, 0, getTerrainDimension() - 1, getTerrainDimension() - 1, TERRAIN_ROUGHNESS);
	}

};
